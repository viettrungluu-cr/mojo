// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/window_manager_impl.h"

#include "mojo/aura/window_tree_host_mojo.h"
#include "mojo/converters/input_events/input_events_type_converters.h"
#include "mojo/services/window_manager/window_manager_app.h"
#include "mojo/services/window_manager/window_manager_delegate.h"

namespace mojo {

WindowManagerImpl::WindowManagerImpl(WindowManagerApp* app) : app_(app) {
}

WindowManagerImpl::~WindowManagerImpl() {
}

void WindowManagerImpl::Embed(
    const String& url,
    InterfaceRequest<ServiceProvider> service_provider) {
  app_->Embed(url, service_provider.Pass());
}

void WindowManagerImpl::OnConnectionEstablished() {
}

}  // namespace mojo
