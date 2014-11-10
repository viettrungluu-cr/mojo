// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_JS_JS_APP_H_
#define MOJO_SERVICES_JS_JS_APP_H_

#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"
#include "gin/public/isolate_holder.h"
#include "gin/shell_runner.h"
#include "mojo/application/content_handler_factory.h"
#include "mojo/edk/js/mojo_runner_delegate.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/interfaces/application/application.mojom.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "mojo/services/public/interfaces/network/url_loader.mojom.h"

namespace mojo {
namespace js {

class JSApp;
class ApplicationDelegateImpl;

// Each JavaScript app started by content handler runs on its own thread and
// in its own V8 isolate. This class represents one running JS app.

class JSApp : public InterfaceImpl<Application>,
              public ContentHandlerFactory::HandledApplicationHolder {
 public:
  JSApp(ShellPtr shell, URLResponsePtr response);
  virtual ~JSApp();

  // Called by the JS mojo module to quit this JS app. See mojo.js.
  void Quit();

  // Called by the JS mojo module to connect to a Mojo application.
  MessagePipeHandle ConnectToApplication(const std::string& application_url);

  // Called by the JS mojo module to retrieve the ServiceProvider message
  // pipe handle passed to the JS application's AcceptConnection() method.
  MessagePipeHandle RequestorMessagePipeHandle();

 private:
  // Application methods:
  void AcceptConnection(const String& requestor_url,
                        ServiceProviderPtr provider) override;
  void Initialize(Array<String> args) override;

  void QuitInternal();

  ShellPtr shell_;
  js::MojoRunnerDelegate runner_delegate;
  gin::IsolateHolder isolate_holder_;
  scoped_ptr<gin::ShellRunner> shell_runner_;
  std::string source_;
  std::string file_name_;
  ScopedMessagePipeHandle requestor_handle_;

  DISALLOW_COPY_AND_ASSIGN(JSApp);
};

}  // namespace js
}  // namespace mojo

#endif  // MOJO_SERVICES_JS_JS_APP_H_
