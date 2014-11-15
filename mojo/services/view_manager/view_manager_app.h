// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_
#define MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/interface_factory.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/error_handler.h"
#include "mojo/services/public/interfaces/window_manager/window_manager_internal.mojom.h"
#include "mojo/services/view_manager/connection_manager_delegate.h"

namespace mojo {
namespace service {

class ConnectionManager;

class ViewManagerApp : public ApplicationDelegate,
                       public ConnectionManagerDelegate,
                       public ErrorHandler,
                       public InterfaceFactory<ViewManagerService>,
                       public InterfaceFactory<WindowManagerInternalClient> {
 public:
  ViewManagerApp();
  ~ViewManagerApp() override;

 private:
  // ApplicationDelegate:
  bool ConfigureIncomingConnection(ApplicationConnection* connection) override;

  // ConnectionManagerDelegate:
  void OnLostConnectionToWindowManager() override;
  ClientConnection* CreateClientConnectionForEmbedAtView(
      ConnectionManager* connection_manager,
      ConnectionSpecificId creator_id,
      const std::string& creator_url,
      const std::string& url,
      const ViewId& root_id) override;

  // InterfaceFactory<ViewManagerService>:
  void Create(ApplicationConnection* connection,
              InterfaceRequest<ViewManagerService> request) override;

  // InterfaceFactory<WindowManagerInternalClient>:
  void Create(ApplicationConnection* connection,
              InterfaceRequest<WindowManagerInternalClient> request) override;

  // ErrorHandler (for |wm_internal_| and |wm_internal_client_binding_|).
  void OnConnectionError() override;

  ApplicationConnection* wm_app_connection_;
  scoped_ptr<Binding<WindowManagerInternalClient>> wm_internal_client_binding_;
  WindowManagerInternalPtr wm_internal_;
  scoped_ptr<ConnectionManager> connection_manager_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerApp);
};

}  // namespace service
}  // namespace mojo

#endif  // MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_
