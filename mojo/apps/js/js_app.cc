// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/apps/js/js_app.h"

#include "base/bind.h"
#include "gin/array_buffer.h"
#include "gin/converter.h"
#include "mojo/apps/js/mojo_bridge_module.h"
#include "mojo/common/data_pipe_utils.h"

namespace mojo {
namespace apps {

JSApp::JSApp(ShellPtr shell, URLResponsePtr response) : shell_(shell.Pass()) {
  // TODO(hansmuller): handle load failure here and below.
  DCHECK(!response.is_null());
  file_name_ = response->url;
  bool result = common::BlockingCopyToString(response->body.Pass(), &source_);
  DCHECK(result);

  runner_delegate.AddBuiltinModule(MojoInternals::kModuleName,
                                   base::Bind(MojoInternals::GetModule, this));
  shell_.set_client(this);
}

JSApp::~JSApp() {
}

void JSApp::Quit() {
  isolate_holder_.RemoveRunMicrotasksObserver();
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&JSApp::QuitInternal, base::Unretained(this)));
}

MessagePipeHandle JSApp::ConnectToApplication(
    const std::string& application_url) {
  MessagePipe pipe;
  InterfaceRequest<ServiceProvider> request =
      MakeRequest<ServiceProvider>(pipe.handle1.Pass());
  shell_->ConnectToApplication(application_url, request.Pass());
  return pipe.handle0.Pass().release();
}

MessagePipeHandle JSApp::RequestorMessagePipeHandle() {
  return requestor_handle_.get();
}

void JSApp::AcceptConnection(const String& requestor_url,
                             ServiceProviderPtr provider) {
  requestor_handle_ = provider.PassMessagePipe();

  isolate_holder_.AddRunMicrotasksObserver();
  shell_runner_.reset(
      new gin::ShellRunner(&runner_delegate, isolate_holder_.isolate()));
  gin::Runner::Scope scope(shell_runner_.get());
  shell_runner_->Run(source_.c_str(), file_name_.c_str());
}

void JSApp::Initialize(Array<String> args) {
}

void JSApp::QuitInternal() {
  shell_runner_.reset();
  base::MessageLoop::current()->QuitWhenIdle();
}

}  // namespace apps
}  // namespace mojo
