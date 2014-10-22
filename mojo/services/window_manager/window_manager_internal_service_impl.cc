// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/window_manager_internal_service_impl.h"

#include "mojo/aura/window_tree_host_mojo.h"
#include "mojo/converters/input_events/input_events_type_converters.h"
#include "mojo/services/window_manager/window_manager_app.h"
#include "mojo/services/window_manager/window_manager_delegate.h"

namespace mojo {

WindowManagerInternalServiceImpl::WindowManagerInternalServiceImpl(
    WindowManagerApp* app)
    : app_(app) {
}

WindowManagerInternalServiceImpl::~WindowManagerInternalServiceImpl() {
}

void WindowManagerInternalServiceImpl::OnViewInputEvent(mojo::EventPtr event) {
  scoped_ptr<ui::Event> ui_event = event.To<scoped_ptr<ui::Event>>();
  if (ui_event)
    app_->host()->SendEventToProcessor(ui_event.get());
}

void WindowManagerInternalServiceImpl::OnConnectionEstablished() {
  app_->set_window_manager_client(client());
}

}  // namespace mojo
