// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/shell/in_process_dynamic_service_runner.h"

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/location.h"
#include "base/message_loop/message_loop_proxy.h"

namespace mojo {
namespace shell {

InProcessDynamicServiceRunner::InProcessDynamicServiceRunner(
    Context* context) : app_library_(nullptr) {
}

InProcessDynamicServiceRunner::~InProcessDynamicServiceRunner() {
  if (thread_) {
    DCHECK(thread_->HasBeenStarted());
    DCHECK(!thread_->HasBeenJoined());
    thread_->Join();
  }

  // It is important to let the thread exit before unloading the DSO because
  // the library may have registered thread-local data and destructors to run
  // on thread termination.
  if (app_library_)
    base::UnloadNativeLibrary(app_library_);
}

void InProcessDynamicServiceRunner::Start(
    const base::FilePath& app_path,
    ScopedMessagePipeHandle service_handle,
    const base::Closure& app_completed_callback) {
  app_path_ = app_path;

  DCHECK(!service_handle_.is_valid());
  service_handle_ = service_handle.Pass();

  DCHECK(app_completed_callback_runner_.is_null());
  app_completed_callback_runner_ = base::Bind(&base::TaskRunner::PostTask,
                                              base::MessageLoopProxy::current(),
                                              FROM_HERE,
                                              app_completed_callback);

  DCHECK(!thread_);
  thread_.reset(new base::DelegateSimpleThread(this, "app_thread"));
  thread_->Start();
}

void InProcessDynamicServiceRunner::Run() {
  DVLOG(2) << "Loading/running Mojo app in process from library: "
           << app_path_.value();

  app_library_ = LoadAndRunService(app_path_, service_handle_.Pass());
  app_completed_callback_runner_.Run();
  app_completed_callback_runner_.Reset();
}

}  // namespace shell
}  // namespace mojo
