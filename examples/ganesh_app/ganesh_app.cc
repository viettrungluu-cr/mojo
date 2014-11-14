// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "examples/ganesh_app/ganesh_view.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/public/cpp/view_manager/view_manager.h"
#include "mojo/services/public/cpp/view_manager/view_manager_client_factory.h"
#include "mojo/services/public/cpp/view_manager/view_manager_delegate.h"

namespace examples {

class GaneshApp : public mojo::ApplicationDelegate,
                  public mojo::ViewManagerDelegate {
 public:
  GaneshApp() {}
  virtual ~GaneshApp() {}

  void Initialize(mojo::ApplicationImpl* app) override {
    shell_ = app->shell();
    view_manager_client_factory_.reset(
        new mojo::ViewManagerClientFactory(app->shell(), this));
  }

  bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override {
    connection->AddService(view_manager_client_factory_.get());
    return true;
  }

  void OnEmbed(mojo::ViewManager* view_manager,
               mojo::View* root,
               mojo::ServiceProviderImpl* exported_services,
               scoped_ptr<mojo::ServiceProvider> imported_services) override {
    new GaneshView(shell_, root);
  }

  void OnViewManagerDisconnected(mojo::ViewManager* view_manager) override {
    base::MessageLoop::current()->Quit();
  }

 private:
  mojo::Shell* shell_;
  scoped_ptr<mojo::ViewManagerClientFactory> view_manager_client_factory_;

  DISALLOW_COPY_AND_ASSIGN(GaneshApp);
};

}  // namespace examples

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new examples::GaneshApp);
  return runner.Run(shell_handle);
}
