// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/application_manager/background_shell_application_loader.h"

#include "base/bind.h"
#include "base/run_loop.h"
#include "mojo/application_manager/application_manager.h"

namespace mojo {

BackgroundShellApplicationLoader::BackgroundShellApplicationLoader(
    scoped_ptr<ApplicationLoader> real_loader,
    const std::string& thread_name,
    base::MessageLoop::Type message_loop_type)
    : loader_(real_loader.Pass()),
      message_loop_type_(message_loop_type),
      thread_name_(thread_name),
      message_loop_created_(true, false) {
}

BackgroundShellApplicationLoader::~BackgroundShellApplicationLoader() {
  if (thread_)
    thread_->Join();
}

void BackgroundShellApplicationLoader::Load(
    ApplicationManager* manager,
    const GURL& url,
    ScopedMessagePipeHandle shell_handle,
    LoadCallback callback) {
  DCHECK(shell_handle.is_valid());
  if (!thread_) {
    // TODO(tim): It'd be nice if we could just have each Load call
    // result in a new thread like DynamicService{Loader, Runner}. But some
    // loaders are creating multiple ApplicationImpls (NetworkApplicationLoader)
    // sharing a delegate (etc). So we have to keep it single threaded, wait
    // for the thread to initialize, and post to the TaskRunner for subsequent
    // Load calls for now.
    thread_.reset(new base::DelegateSimpleThread(this, thread_name_));
    thread_->Start();
    message_loop_created_.Wait();
    DCHECK(task_runner_.get());
  }

  task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&BackgroundShellApplicationLoader::LoadOnBackgroundThread,
                 base::Unretained(this), manager, url,
                 base::Passed(&shell_handle)));
}

void BackgroundShellApplicationLoader::OnApplicationError(
    ApplicationManager* manager,
    const GURL& url) {
  task_runner_->PostTask(FROM_HERE,
                         base::Bind(&BackgroundShellApplicationLoader::
                                        OnApplicationErrorOnBackgroundThread,
                                    base::Unretained(this),
                                    manager,
                                    url));
}

void BackgroundShellApplicationLoader::Run() {
  base::MessageLoop message_loop(message_loop_type_);
  base::RunLoop loop;
  task_runner_ = message_loop.task_runner();
  quit_closure_ = loop.QuitClosure();
  message_loop_created_.Signal();
  loop.Run();

  // Destroy |loader_| on the thread it's actually used on.
  loader_.reset();
}

void BackgroundShellApplicationLoader::LoadOnBackgroundThread(
    ApplicationManager* manager,
    const GURL& url,
    ScopedMessagePipeHandle shell_handle) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  loader_->Load(manager, url, shell_handle.Pass(), SimpleLoadCallback());
}

void BackgroundShellApplicationLoader::OnApplicationErrorOnBackgroundThread(
    ApplicationManager* manager,
    const GURL& url) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  loader_->OnApplicationError(manager, url);
}

}  // namespace mojo
