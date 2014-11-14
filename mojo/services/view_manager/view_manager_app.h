// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_
#define MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/services/view_manager/connection_manager_delegate.h"

namespace mojo {
namespace service {

class ConnectionManager;

class ViewManagerApp : public ApplicationDelegate,
                       public ConnectionManagerDelegate {
 public:
  ViewManagerApp();
  ~ViewManagerApp() override;

 private:
  // ApplicationDelegate:
  bool ConfigureIncomingConnection(ApplicationConnection* connection) override;

  // ConnectionManagerDelegate:
  void OnLostConnectionToWindowManager() override;

  scoped_ptr<ConnectionManager> connection_manager_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerApp);
};

}  // namespace service
}  // namespace mojo

#endif  // MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_APP_H_
