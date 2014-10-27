// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_INTERNAL_SERVICE_IMPL_H_
#define MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_INTERNAL_SERVICE_IMPL_H_

#include "base/basictypes.h"
#include "mojo/services/public/interfaces/window_manager/window_manager_internal.mojom.h"

namespace mojo {

class WindowManagerApp;

class WindowManagerInternalServiceImpl
    : public InterfaceImpl<WindowManagerInternalService> {
 public:
  explicit WindowManagerInternalServiceImpl(WindowManagerApp* app);
  virtual ~WindowManagerInternalServiceImpl();

 private:
  // WindowManagerInternalServiceImpl:
  virtual void OnViewInputEvent(mojo::EventPtr event) override;

  // InterfaceImpl:
  virtual void OnConnectionEstablished() override;

  WindowManagerApp* app_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerInternalServiceImpl);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_INTERNAL_SERVICE_IMPL_H_
