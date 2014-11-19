// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_
#define MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_

#include "base/basictypes.h"
#include "base/logging.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/error_handler.h"
#include "mojo/services/public/cpp/view_manager/types.h"
#include "mojo/services/public/interfaces/window_manager/window_manager.mojom.h"

namespace mojo {

class WindowManagerApp;

class WindowManagerImpl : public WindowManager, public ErrorHandler {
 public:
  // See description above |from_vm_| for details on |from_vm|.
  // WindowManagerImpl deletes itself on connection errors.  WindowManagerApp
  // also deletes WindowManagerImpl in its destructor.
  WindowManagerImpl(WindowManagerApp* window_manager, bool from_vm);
  ~WindowManagerImpl() override;

  void Bind(ScopedMessagePipeHandle window_manager_pipe);

  void NotifyViewFocused(Id new_focused_id, Id old_focused_id);
  void NotifyWindowActivated(Id new_active_id, Id old_active_id);

 private:
  WindowManagerClient* client() {
    DCHECK(from_vm_);
    return binding_.client();
  }

  // WindowManager:
  void Embed(const String& url,
             InterfaceRequest<ServiceProvider> service_provider) override;
  void SetCapture(uint32_t view_id,
                  const Callback<void(bool)>& callback) override;
  void FocusWindow(uint32_t view_id,
                   const Callback<void(bool)>& callback) override;
  void ActivateWindow(uint32_t view_id,
                      const Callback<void(bool)>& callback) override;

  // ErrorHandler:
  void OnConnectionError() override;

  WindowManagerApp* window_manager_;

  // Whether this connection originated from the ViewManager. Connections that
  // originate from the view manager are expected to have clients. Connections
  // that don't originate from the view manager do not have clients.
  const bool from_vm_;

  Binding<WindowManager> binding_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerImpl);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_IMPL_H_
