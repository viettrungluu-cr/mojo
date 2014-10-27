// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/view_manager/connection_manager.h"
#include "mojo/services/view_manager/connection_manager_delegate.h"

namespace mojo {
namespace service {

class ViewManagerApp : public ApplicationDelegate,
                       public ConnectionManagerDelegate {
 public:
  ViewManagerApp() {}
  ~ViewManagerApp() override {}

  bool ConfigureIncomingConnection(ApplicationConnection* connection) override {
    if (!connection_manager_.get()) {
      connection_manager_.reset(new ConnectionManager(connection, this));
      return true;
    }
    VLOG(1) << "ViewManager allows only one connection.";
    return false;
  }

 private:
  // ConnectionManagerDelegate:
  void OnNativeViewportDestroyed() override { ApplicationImpl::Terminate(); }
  void OnLostConnectionToWindowManager() override {
    ApplicationImpl::Terminate();
  }

  scoped_ptr<ConnectionManager> connection_manager_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerApp);
};

}  // namespace service
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::service::ViewManagerApp);
  return runner.Run(shell_handle);
}
