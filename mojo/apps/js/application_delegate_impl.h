// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_APPS_JS_CONTENT_HANDLER_H_
#define MOJO_APPS_JS_CONTENT_HANDLER_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/bindings/interface_request.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"

namespace mojo {

class ApplcationImpl;

namespace apps {

class ApplicationDelegateImpl;
class JSApp;

// Manages the JSApps started by this content handler. This class owns the one
// reference to the Mojo shell. JSApps post a task to the content handler's
// thread to connect to a service or to quit.
//
// The lifetime each JSApp is defined by its entry in AppVector. When the entry
// is removed ("erased") by QuitJSApp(), the JSApp is destroyed.

class ApplicationDelegateImpl : public ApplicationDelegate {
 public:
  ApplicationDelegateImpl();
  ~ApplicationDelegateImpl() override;

  // Add app to the AppVector and call its Start() method.
  void StartJSApp(scoped_ptr<JSApp> app);

  // Remove app from the AppVector; destroys the app.
  void QuitJSApp(JSApp *app);

  // Use the shell to connect to a ServiceProvider for application_url.
  void ConnectToApplication(const std::string& application_url,
                            InterfaceRequest<ServiceProvider> request);

 protected:
  // ApplicationDelegate:
  void Initialize(ApplicationImpl* app) override;

 private:
  typedef ScopedVector<JSApp> AppVector;
  ApplicationImpl* application_impl_;  // Owns the shell used by all JSApps.
  AppVector app_vector_;
};

}  // namespace apps
}  // namespace mojo

#endif  // MOJO_APPS_JS_CONTENT_HANDLER_H_
