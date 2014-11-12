// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "base/memory/scoped_ptr.h"
#include "dart/runtime/include/dart_api.h"
#include "mojo/edk/embedder/embedder.h"
#include "mojo/edk/embedder/platform_support.h"
#include "mojo/edk/embedder/simple_platform_support.h"
#include "mojo/public/platform/native/system_thunks.h"

static Dart_NativeFunction ResolveName(
    Dart_Handle name, int argc, bool* auto_setup_scope);


DART_EXPORT Dart_Handle mojo_dart_init_Init(Dart_Handle parent_library) {
  if (Dart_IsError(parent_library)) {
    return parent_library;
  }

  Dart_Handle result_code =
      Dart_SetNativeResolver(parent_library, ResolveName, NULL);
  if (Dart_IsError(result_code)) {
    return result_code;
  }

  return Dart_Null();
}


static Dart_Handle HandleError(Dart_Handle handle) {
  if (Dart_IsError(handle)) {
    Dart_PropagateError(handle);
  }
  return handle;
}


static void MojoLibrary_Init(Dart_NativeArguments arguments) {
  mojo::embedder::Init(scoped_ptr<mojo::embedder::PlatformSupport>(
      new mojo::embedder::SimplePlatformSupport()));
  Dart_SetReturnValue(arguments, Dart_Null());
}


static void MojoSystemThunks_Make(Dart_NativeArguments arguments) {
  Dart_Handle closure = HandleError(Dart_GetNativeArgument(arguments, 0));

  if (Dart_IsClosure(closure)) {
    MojoSystemThunks thunks = MojoMakeSystemThunks();
    int64_t thunks_addr = reinterpret_cast<int64_t>(&thunks);

    Dart_Handle t = Dart_NewInteger(thunks_addr);
    Dart_InvokeClosure(closure, 1, &t);
  }

  Dart_SetReturnValue(arguments, Dart_Null());
}


#define SCOPE_FUNCTIONS(V)                                                     \
  V(MojoSystemThunks_Make)                                                     \

#define NOSCOPE_FUNCTIONS(V)                                                   \
  V(MojoLibrary_Init)                                                          \


struct FunctionLookup {
  const char* name;
  Dart_NativeFunction function;
};


#define FUNCTION_STRING_MAP(name) {#name, name},

FunctionLookup function_list[] = {
  SCOPE_FUNCTIONS(FUNCTION_STRING_MAP)
  {NULL, NULL}
};

FunctionLookup no_scope_function_list[] = {
  NOSCOPE_FUNCTIONS(FUNCTION_STRING_MAP)
  {NULL, NULL}
};

#undef FUNCTION_STRING_MAP


Dart_NativeFunction ResolveName(Dart_Handle name,
                                int argc,
                                bool* auto_setup_scope) {
  if (!Dart_IsString(name)) {
    return NULL;
  }
  Dart_NativeFunction result = NULL;
  if (auto_setup_scope == NULL) {
    return NULL;
  }

  Dart_EnterScope();
  const char* cname;
  HandleError(Dart_StringToCString(name, &cname));

  for (int i=0; function_list[i].name != NULL; ++i) {
    if (strcmp(function_list[i].name, cname) == 0) {
      *auto_setup_scope = true;
      Dart_ExitScope();
      return function_list[i].function;
    }
  }

  for (int i=0; no_scope_function_list[i].name != NULL; ++i) {
    if (strcmp(no_scope_function_list[i].name, cname) == 0) {
      *auto_setup_scope = false;
      result = no_scope_function_list[i].function;
      break;
    }
  }

  Dart_ExitScope();
  return result;
}
