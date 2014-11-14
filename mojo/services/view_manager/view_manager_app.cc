// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/view_manager/view_manager_app.h"

#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/view_manager/connection_manager.h"
#include "mojo/services/view_manager/display_manager.h"

namespace mojo {
namespace service {

ViewManagerApp::ViewManagerApp() {}
ViewManagerApp::~ViewManagerApp() {}

bool ViewManagerApp::ConfigureIncomingConnection(
    ApplicationConnection* connection) {
  if (!connection_manager_.get()) {
    scoped_ptr<DefaultDisplayManager> display_manager(
        new DefaultDisplayManager(
            connection,
            base::Bind(&ViewManagerApp::OnLostConnectionToWindowManager,
                       base::Unretained(this))));
    connection_manager_.reset(
        new ConnectionManager(connection, this, display_manager.Pass()));
    return true;
  }
  VLOG(1) << "ViewManager allows only one connection.";
  return false;
}

void ViewManagerApp::OnLostConnectionToWindowManager() {
  ApplicationImpl::Terminate();
}

}  // namespace service
}  // namespace mojo
