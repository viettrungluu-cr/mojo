// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_VIEW_MANAGER_CONNECTION_MANAGER_DELEGATE_H_
#define MOJO_SERVICES_VIEW_MANAGER_CONNECTION_MANAGER_DELEGATE_H_

#include <string>

#include "mojo/services/public/cpp/view_manager/types.h"

namespace mojo {

class ViewManagerService;

namespace service {

class ClientConnection;
class ConnectionManager;
struct ViewId;

class ConnectionManagerDelegate {
 public:
  virtual void OnLostConnectionToWindowManager() = 0;

  // Creates a ClientConnection in response to Embed() calls on the
  // ConnectionManager.
  virtual ClientConnection* CreateClientConnectionForEmbedAtView(
      ConnectionManager* connection_manager,
      ConnectionSpecificId creator_id,
      const std::string& creator_url,
      const std::string& url,
      const ViewId& root_id) = 0;

 protected:
  virtual ~ConnectionManagerDelegate() {}
};

}  // namespace service
}  // namespace mojo

#endif  // MOJO_SERVICES_VIEW_MANAGER_CONNECTION_MANAGER_DELEGATE_H_
