// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_
#define MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_

#include "base/basictypes.h"
#include "mojo/services/public/cpp/view_manager/types.h"
#include "mojo/services/public/interfaces/window_manager/window_manager.mojom.h"

namespace mojo {

class WindowManagerApp;

class WindowManagerImpl : public InterfaceImpl<WindowManager> {
 public:
  // The constructor without |from_vm| is necessitated by
  // InterfaceFactoryImplWithContext. It create a WindowManagerImpl with
  // |from_vm| equal to false.
  explicit WindowManagerImpl(WindowManagerApp* window_manager);
  WindowManagerImpl(WindowManagerApp* window_manager, bool from_vm);
  ~WindowManagerImpl() override;

  void NotifyViewFocused(Id new_focused_id, Id old_focused_id);
  void NotifyWindowActivated(Id new_active_id, Id old_active_id);

 private:
  // WindowManager:
  void Embed(const String& url,
             InterfaceRequest<ServiceProvider> service_provider) override;
  void SetCapture(uint32_t view_id,
                  const Callback<void(bool)>& callback) override;
  void FocusWindow(uint32_t view_id,
                   const Callback<void(bool)>& callback) override;
  void ActivateWindow(uint32_t view_id,
                      const Callback<void(bool)>& callback) override;

  // InterfaceImpl:
  void OnConnectionEstablished() override;

  WindowManagerApp* window_manager_;

  // Whether this connection originated from the ViewManager. Connections that
  // originate from the view manager are expected to have clients. Connections
  // that don't originate from the view manager do not have clients.
  const bool from_vm_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerImpl);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_
