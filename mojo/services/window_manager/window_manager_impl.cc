// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/window_manager_impl.h"

#include "mojo/services/window_manager/window_manager_app.h"

namespace mojo {

WindowManagerImpl::WindowManagerImpl(WindowManagerApp* window_manager,
                                     bool from_vm)
    : window_manager_(window_manager), from_vm_(from_vm), binding_(this) {
  window_manager_->AddConnection(this);
  binding_.set_error_handler(this);
}

WindowManagerImpl::~WindowManagerImpl() {
  window_manager_->RemoveConnection(this);
}

void WindowManagerImpl::Bind(ScopedMessagePipeHandle window_manager_pipe) {
  binding_.Bind(window_manager_pipe.Pass());
}

void WindowManagerImpl::NotifyViewFocused(Id new_focused_id,
                                          Id old_focused_id) {
  if (from_vm_)
    client()->OnFocusChanged(old_focused_id, new_focused_id);
}

void WindowManagerImpl::NotifyWindowActivated(Id new_active_id,
                                              Id old_active_id) {
  if (from_vm_)
    client()->OnActiveWindowChanged(old_active_id, new_active_id);
}

void WindowManagerImpl::Embed(
    const String& url,
    InterfaceRequest<ServiceProvider> service_provider) {
  window_manager_->Embed(url, service_provider.Pass());
}

void WindowManagerImpl::SetCapture(Id view,
                                   const Callback<void(bool)>& callback) {
  if (!from_vm_)
    return;  // See comments for |from_vm_| on this.

  bool success = window_manager_->IsReady();
  if (success)
    window_manager_->SetCapture(view);
  callback.Run(success);
}

void WindowManagerImpl::FocusWindow(Id view,
                                    const Callback<void(bool)>& callback) {
  if (!from_vm_)
    return;  // See comments for |from_vm_| on this.

  bool success = window_manager_->IsReady();
  if (success)
    window_manager_->FocusWindow(view);
  callback.Run(success);
}

void WindowManagerImpl::ActivateWindow(Id view,
                                       const Callback<void(bool)>& callback) {
  if (!from_vm_)
    return;  // See comments for |from_vm_| on this.

  bool success = window_manager_->IsReady();
  if (success)
    window_manager_->ActivateWindow(view);
  callback.Run(success);
}

void WindowManagerImpl::OnConnectionError() {
  delete this;
}

}  // namespace mojo
