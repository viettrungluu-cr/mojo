// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/js/js_app.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "gin/converter.h"
#include "gin/modules/module_registry.h"
#include "gin/try_catch.h"
#include "mojo/common/data_pipe_utils.h"
#include "mojo/edk/js/core.h"
#include "mojo/edk/js/handle.h"
#include "mojo/edk/js/support.h"
#include "mojo/public/cpp/bindings/interface_request.h"
#include "services/js/js_app_shell.h"
#include "services/js/mojo_bridge_module.h"

namespace mojo {
namespace js {

const char JSApp::kMainModuleName[] = "main";

JSApp::JSApp(ShellPtr shell, URLResponsePtr response) : shell_(shell.Pass()) {
  isolate_holder_.AddRunMicrotasksObserver();

  DCHECK(!response.is_null());
  std::string url(response->url);
  std::string source;
  CHECK(common::BlockingCopyToString(response->body.Pass(), &source));

  v8::Isolate* isolate = isolate_holder_.isolate();
  shell_runner_.reset(new gin::ShellRunner(&runner_delegate_, isolate));
  gin::Runner::Scope scope(shell_runner_.get());
  shell_runner_->Run(source.c_str(), kMainModuleName);

  gin::ModuleRegistry* registry =
      gin::ModuleRegistry::From(shell_runner_->GetContextHolder()->context());
  registry->LoadModule(
      isolate,
      kMainModuleName,
      base::Bind(&JSApp::OnAppLoaded, base::Unretained(this), url));
}

JSApp::~JSApp() {
  app_instance_.Reset();
}


void JSApp::OnAppLoaded(std::string url, v8::Handle<v8::Value> main_module) {
  gin::Runner::Scope scope(shell_runner_.get());
  gin::TryCatch try_catch;
  v8::Isolate* isolate = isolate_holder_.isolate();

  v8::Handle<v8::Value> argv[] = {
    gin::ConvertToV8(isolate, JSAppShell::Create(isolate, this)),
    gin::ConvertToV8(isolate, url)
  };

  v8::Handle<v8::Function> app_class;
  CHECK(gin::ConvertFromV8(isolate, main_module, &app_class));
  app_instance_.Reset(isolate, app_class->NewInstance(arraysize(argv), argv));
  if (try_catch.HasCaught())
    runner_delegate_.UnhandledException(shell_runner_.get(), try_catch);

  shell_.set_client(this);
}

void JSApp::ConnectToApplication(const std::string& application_url,
                                 ScopedMessagePipeHandle service_provider) {
  shell_->ConnectToApplication(
      application_url, MakeRequest<ServiceProvider>(service_provider.Pass()));
}

void JSApp::CallAppInstanceMethod(
    const std::string& name, int argc, v8::Handle<v8::Value> argv[]) {
  v8::Isolate* isolate = isolate_holder_.isolate();
  v8::Local<v8::Object> app =
      v8::Local<v8::Object>::New(isolate, app_instance_);
  v8::Handle<v8::Value> key = gin::StringToV8(isolate, name);
  v8::Handle<v8::Value> value = app->Get(key);
  if (!value->IsFunction())
    return;
  v8::Handle<v8::Function> app_method;
  CHECK(gin::ConvertFromV8(isolate, value, &app_method));
  shell_runner_->Call(app_method, app, argc, argv);
}

void JSApp::Initialize(Array<String> app_args) {
  gin::Runner::Scope scope(shell_runner_.get());
  v8::Isolate* isolate = isolate_holder_.isolate();
  v8::Handle<v8::Value> argv[] = {
    gin::ConvertToV8(isolate, app_args.To<std::vector<std::string>>()),
  };
  CallAppInstanceMethod("initialize", 1, argv);
}

void JSApp::AcceptConnection(const String& requestor_url,
                             ServiceProviderPtr provider) {
  gin::Runner::Scope scope(shell_runner_.get());
  v8::Isolate* isolate = isolate_holder_.isolate();
  v8::Handle<v8::Value> argv[] = {
    gin::ConvertToV8(isolate, requestor_url.To<std::string>()),
    gin::ConvertToV8(isolate, provider.PassMessagePipe().get()),
  };
  CallAppInstanceMethod("acceptConnection", arraysize(argv), argv);
}

void JSApp::Quit() {
  isolate_holder_.RemoveRunMicrotasksObserver();
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&JSApp::QuitInternal, base::Unretained(this)));
}

void JSApp::QuitInternal() {
  shell_runner_.reset();
  base::MessageLoop::current()->QuitWhenIdle();
}

}  // namespace js
}  // namespace mojo

