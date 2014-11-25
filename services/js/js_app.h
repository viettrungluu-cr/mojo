// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_JS_JS_APP_H_
#define SERVICES_JS_JS_APP_H_

#include "gin/public/isolate_holder.h"
#include "gin/shell_runner.h"
#include "mojo/application/content_handler_factory.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "mojo/public/interfaces/application/application.mojom.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "services/js/js_app_runner_delegate.h"
#include "v8/include/v8.h"

namespace mojo {
namespace js {

class JSApp;
class ApplicationDelegateImpl;

// Each JavaScript app started by the content handler runs on its own thread
// and in its own V8 isolate. This class represents one running JS app.

class JSApp : public InterfaceImpl<Application>,
              public ContentHandlerFactory::HandledApplicationHolder {
 public:
  JSApp(ShellPtr shell, URLResponsePtr response);
  virtual ~JSApp();

  // This method just delegates to shell_->ConnectToApplication().
  void ConnectToApplication(const std::string& application_url,
                            ScopedMessagePipeHandle service_provider);

  void Quit();

 private:
  static const char kMainModuleName[];

  void OnAppLoaded(std::string url, v8::Handle<v8::Value> module);

  // Application methods:
  void AcceptConnection(const String& requestor_url,
                        ServiceProviderPtr provider) override;
  void Initialize(Array<String> args) override;

  void CallAppInstanceMethod(
      const std::string& name, int argc, v8::Handle<v8::Value> argv[]);

  void QuitInternal();

  ShellPtr shell_;
  JSAppRunnerDelegate runner_delegate_;
  gin::IsolateHolder isolate_holder_;
  scoped_ptr<gin::ShellRunner> shell_runner_;
  v8::Persistent<v8::Object> app_instance_;

  DISALLOW_COPY_AND_ASSIGN(JSApp);
};

}  // namespace js
}  // namespace mojo

#endif  // SERVICES_JS_JS_APP_H_
