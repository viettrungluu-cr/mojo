// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/view_target.h"

#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/window_manager/window_manager_app.h"
#include "ui/events/event.h"
#include "ui/events/event_target_iterator.h"
#include "ui/events/event_targeter.h"

namespace mojo {

class ViewTargeter : public ui::EventTargeter {};

ViewTarget::ViewTarget(WindowManagerApp* app, View* view_to_wrap)
    : app_(app),
      view_(view_to_wrap) {
}

ViewTarget::~ViewTarget() {
  // We don't own our children or |view_|.
}

bool ViewTarget::HasParent() const {
  return view_->parent();
}

bool ViewTarget::IsVisible() const {
  return view_->visible();
}

void ViewTarget::AddChild(ViewTarget* view) {
  children_.push_back(view);
}

scoped_ptr<ViewTargeter> ViewTarget::SetEventTargeter(
    scoped_ptr<ViewTargeter> targeter) {
  scoped_ptr<ViewTargeter> old_targeter = targeter_.Pass();
  targeter_ = targeter.Pass();
  return old_targeter.Pass();
}

bool ViewTarget::CanAcceptEvent(const ui::Event& event) {
  // We need to make sure that a touch cancel event and any gesture events it
  // creates can always reach the window. This ensures that we receive a valid
  // touch / gesture stream.
  if (event.IsEndingEvent())
    return true;

  if (!view_->visible())
    return false;

  // The top-most window can always process an event.
  if (!view_->parent())
    return true;

  // For located events (i.e. mouse, touch etc.), an assumption is made that
  // windows that don't have a default event-handler cannot process the event
  // (see more in GetWindowForPoint()). This assumption is not made for key
  // events.
  return event.IsKeyEvent() || target_handler();
}

ui::EventTarget* ViewTarget::GetParentTarget() {
  if (!view_->parent()) {
    // We are the root node.
    return nullptr;
  }

  return app_->GetViewTargetForViewId(view_->parent()->id());
}

scoped_ptr<ui::EventTargetIterator> ViewTarget::GetChildIterator() const {
  return scoped_ptr<ui::EventTargetIterator>(
      new ui::EventTargetIteratorImpl<ViewTarget>(children_));
}

ui::EventTargeter* ViewTarget::GetEventTargeter() {
  return targeter_.get();
}

void ViewTarget::ConvertEventToTarget(ui::EventTarget* target,
                                      ui::LocatedEvent* event) {
  // TODO(erg): Actually doing enabling this line requires doing some partially
  // specialized template cruft. Punt for now.
  //
  // event->ConvertLocationToTarget(this,
  //                                static_cast<ViewTarget*>(target));
}

}  // namespace mojo
