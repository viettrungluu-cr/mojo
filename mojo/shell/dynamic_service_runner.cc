// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/shell/dynamic_service_runner.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "mojo/public/platform/native/gles2_impl_chromium_sync_point_thunks.h"
#include "mojo/public/platform/native/gles2_impl_chromium_texture_mailbox_thunks.h"
#include "mojo/public/platform/native/gles2_impl_thunks.h"
#include "mojo/public/platform/native/gles2_thunks.h"
#include "mojo/public/platform/native/system_thunks.h"

namespace mojo {
namespace shell {

namespace {

template <typename Thunks>
bool SetThunks(Thunks (*make_thunks)(),
               const char* function_name,
               base::NativeLibrary library) {
  typedef size_t (*SetThunksFn)(const Thunks* thunks);
  SetThunksFn set_thunks =
      reinterpret_cast<SetThunksFn>(base::GetFunctionPointerFromNativeLibrary(
          library, function_name));
  if (!set_thunks)
    return false;
  Thunks thunks = make_thunks();
  size_t expected_size = set_thunks(&thunks);
  if (expected_size > sizeof(Thunks)) {
    LOG(ERROR) << "Invalid app library: expected " << function_name
               << " to return thunks of size: " << expected_size;
    return false;
  }
  return true;
}

}  // namespace

base::NativeLibrary DynamicServiceRunner::LoadAndRunService(
    const base::FilePath& app_path,
    ScopedMessagePipeHandle service_handle) {
  DVLOG(2) << "Loading/running Mojo app in process from library: "
           << app_path.value();
  base::NativeLibraryLoadError error;
  base::NativeLibrary app_library = base::LoadNativeLibrary(app_path, &error);
  do {
    if (!app_library) {
      LOG(ERROR) << "Failed to load app library (error: " << error.ToString()
                 << ")";
      break;
    }
    // Go shared library support requires us to initialize the runtime before we
    // start running any go code. This is a temporary patch.
    typedef void (*InitGoRuntimeFn)();
    InitGoRuntimeFn init_go_runtime = reinterpret_cast<InitGoRuntimeFn>(
        base::GetFunctionPointerFromNativeLibrary(
            app_library, "InitGoRuntime"));
    if (init_go_runtime) {
      DVLOG(2) << "InitGoRuntime: Initializing Go Runtime found in app";
      init_go_runtime();
    }

    if (!SetThunks(&MojoMakeSystemThunks, "MojoSetSystemThunks", app_library)) {
      LOG(ERROR) << app_path.value() << " MojoSetSystemThunks not found";
      break;
    }

    if (SetThunks(&MojoMakeGLES2ControlThunks,
                  "MojoSetGLES2ControlThunks",
                  app_library)) {
      // If we have the control thunks, we should also have the GLES2
      // implementation thunks.
      if (!SetThunks(&MojoMakeGLES2ImplThunks,
                     "MojoSetGLES2ImplThunks",
                     app_library)) {
        LOG(ERROR) << app_path.value()
                   << " has MojoSetGLES2ControlThunks, "
                      "but doesn't have MojoSetGLES2ImplThunks.";
        break;
      }

      // If the application is using GLES2 extension points, register those
      // thunks. Applications may use or not use any of these, so don't warn if
      // they are missing.
      SetThunks(MojoMakeGLES2ImplChromiumTextureMailboxThunks,
                "MojoSetGLES2ImplChromiumTextureMailboxThunks",
                app_library);
      SetThunks(MojoMakeGLES2ImplChromiumSyncPointThunks,
                "MojoSetGLES2ImplChromiumSyncPointThunks",
                app_library);
    }
    // Unlike system thunks, we don't warn on a lack of GLES2 thunks because
    // not everything is a visual app.

    typedef MojoResult (*MojoMainFunction)(MojoHandle);
    MojoMainFunction main_function = reinterpret_cast<MojoMainFunction>(
        base::GetFunctionPointerFromNativeLibrary(app_library, "MojoMain"));
    if (!main_function) {
      LOG(ERROR) << app_path.value() << " MojoMain not found";
      break;
    }
    // |MojoMain()| takes ownership of the service handle.
    MojoResult result = main_function(service_handle.release().value());
    if (result < MOJO_RESULT_OK) {
      LOG(ERROR) << app_path.value()
                 << " MojoMain returned error(" << result << ")";
    }
  } while (false);

  return app_library;
}

}  // namespace shell
}  // namespace mojo
