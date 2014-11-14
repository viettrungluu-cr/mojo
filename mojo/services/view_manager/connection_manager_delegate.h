// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_VIEW_MANAGER_CONNECTION_MANAGER_DELEGATE_H_
#define MOJO_SERVICES_VIEW_MANAGER_CONNECTION_MANAGER_DELEGATE_H_

namespace mojo {
namespace service {

class ConnectionManagerDelegate {
 public:
  virtual void OnLostConnectionToWindowManager() = 0;

 protected:
  virtual ~ConnectionManagerDelegate() {}
};

}  // namespace service
}  // namespace mojo

#endif  // MOJO_SERVICES_VIEW_MANAGER_CONNECTION_MANAGER_DELEGATE_H_
