// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/view_manager/view_manager_app.h"

#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/view_manager/client_connection.h"
#include "mojo/services/view_manager/connection_manager.h"
#include "mojo/services/view_manager/display_manager.h"
#include "mojo/services/view_manager/view_manager_service_impl.h"

namespace mojo {
namespace service {

ViewManagerApp::ViewManagerApp() : wm_app_connection_(nullptr) {
}
ViewManagerApp::~ViewManagerApp() {}

bool ViewManagerApp::ConfigureIncomingConnection(
    ApplicationConnection* connection) {
  if (connection_manager_.get()) {
    VLOG(1) << "ViewManager allows only one window manager connection.";
    return false;
  }
  wm_app_connection_ = connection;
  // |connection| originates from the WindowManager. Let it connect directly
  // to the ViewManager and WindowManagerInternalClient.
  connection->AddService(
      static_cast<InterfaceFactory<ViewManagerService>*>(this));
  connection->AddService(
      static_cast<InterfaceFactory<WindowManagerInternalClient>*>(this));
  connection->ConnectToService(&wm_internal_);
  // TODO(sky): add this back. It's causing tests to hang, figure out why.
  // wm_internal_.set_error_handler(this);

  scoped_ptr<DefaultDisplayManager> display_manager(new DefaultDisplayManager(
      connection, base::Bind(&ViewManagerApp::OnLostConnectionToWindowManager,
                             base::Unretained(this))));
  connection_manager_.reset(
      new ConnectionManager(this, display_manager.Pass(), wm_internal_.get()));
  return true;
}

void ViewManagerApp::OnLostConnectionToWindowManager() {
  ApplicationImpl::Terminate();
}

ClientConnection* ViewManagerApp::CreateClientConnectionForEmbedAtView(
    ConnectionManager* connection_manager,
    ConnectionSpecificId creator_id,
    const std::string& creator_url,
    const std::string& url,
    const ViewId& root_id) {
  MessagePipe pipe;

  ServiceProvider* view_manager_service_provider =
      wm_app_connection_->ConnectToApplication(url)->GetServiceProvider();
  view_manager_service_provider->ConnectToService(
      ViewManagerServiceImpl::Client::Name_, pipe.handle1.Pass());
  scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
      connection_manager, creator_id, creator_url, url, root_id));
  return new DefaultClientConnection(service.Pass(), connection_manager,
                                     pipe.handle0.Pass());
}

void ViewManagerApp::Create(ApplicationConnection* connection,
                            InterfaceRequest<ViewManagerService> request) {
  if (connection_manager_->has_window_manager_client_connection()) {
    VLOG(1) << "ViewManager interface requested more than once.";
    return;
  }

  scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
      connection_manager_.get(), kInvalidConnectionId, std::string(),
      std::string("mojo:window_manager"), RootViewId()));
  scoped_ptr<ClientConnection> client_connection(new DefaultClientConnection(
      service.Pass(), connection_manager_.get(), request.PassMessagePipe()));
  connection_manager_->SetWindowManagerClientConnection(
      client_connection.Pass());
}

void ViewManagerApp::Create(
    ApplicationConnection* connection,
    InterfaceRequest<WindowManagerInternalClient> request) {
  if (wm_internal_client_binding_.get()) {
    VLOG(1) << "WindowManagerInternalClient requested more than once.";
    return;
  }

  // ConfigureIncomingConnection() must have been called before getting here.
  DCHECK(connection_manager_.get());
  wm_internal_client_binding_.reset(new Binding<WindowManagerInternalClient>(
      connection_manager_.get(), request.Pass()));
  wm_internal_client_binding_->set_error_handler(this);
}

void ViewManagerApp::OnConnectionError() {
  ApplicationImpl::Terminate();
}

}  // namespace service
}  // namespace mojo
