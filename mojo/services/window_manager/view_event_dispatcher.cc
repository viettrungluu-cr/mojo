// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/view_event_dispatcher.h"

#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/window_manager/view_target.h"

namespace mojo {

ViewEventDispatcher::ViewEventDispatcher()
    : event_dispatch_target_(nullptr),
      old_dispatch_target_(nullptr) {
}

ViewEventDispatcher::~ViewEventDispatcher() {}

void ViewEventDispatcher::SetRootViewTarget(ViewTarget* root_view_target) {
  root_view_target_ = root_view_target;
}

ui::EventTarget* ViewEventDispatcher::GetRootTarget() {
  return root_view_target_;
}

void ViewEventDispatcher::OnEventProcessingStarted(ui::Event* event) {
  // TODO(erg): In the aura version of this class, we perform a DIP based
  // transformation here. Do we want to do any sort of analogous transformation
  // here.
}

bool ViewEventDispatcher::CanDispatchToTarget(ui::EventTarget* target) {
  return event_dispatch_target_ == target;
}

ui::EventDispatchDetails ViewEventDispatcher::PreDispatchEvent(
    ui::EventTarget* target,
    ui::Event* event) {
  // TODO(erg): PreDispatch in aura::WindowEventDispatcher does many, many
  // things. It, and the functions split off for different event types, are
  // most of the file.
  old_dispatch_target_ = event_dispatch_target_;
  event_dispatch_target_ = static_cast<ViewTarget*>(target);
  return ui::EventDispatchDetails();
}

ui::EventDispatchDetails ViewEventDispatcher::PostDispatchEvent(
    ui::EventTarget* target,
    const ui::Event& event) {
  // TODO(erg): Not at all as long as PreDispatchEvent, but still missing core
  // details.
  event_dispatch_target_ = old_dispatch_target_;
  old_dispatch_target_ = nullptr;
  return ui::EventDispatchDetails();
}

}  // namespace mojo
