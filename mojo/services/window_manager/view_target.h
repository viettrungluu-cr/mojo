// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_VIEW_TARGET_H_
#define MOJO_SERVICES_WINDOW_MANAGER_VIEW_TARGET_H_

#include "ui/events/event_target.h"

namespace ui {
class EventTargeter;
}

namespace mojo {

class View;
class ViewTargeter;
class WindowManagerApp;

// A wrapper class around mojo::View. We maintain a shadow tree of ViewTargets,
// which mirrors the main View tree. The ViewTarget tree wraps mojo::View
// because we can't subclass View to implement the event targeting interfaces.
class ViewTarget : public ui::EventTarget {
 public:
  ViewTarget(WindowManagerApp* app, View* view_to_wrap);
  ~ViewTarget() override;

  View* view() { return view_; }

  bool HasParent() const;
  bool IsVisible() const;

  // We keep track of our children here.
  void AddChild(ViewTarget* view);

  // Sets a new ViewTargeter for the view, and returns the previous
  // ViewTargeter.
  scoped_ptr<ViewTargeter> SetEventTargeter(scoped_ptr<ViewTargeter> targeter);

  // Overridden from ui::EventTarget:
  bool CanAcceptEvent(const ui::Event& event) override;
  EventTarget* GetParentTarget() override;
  scoped_ptr<ui::EventTargetIterator> GetChildIterator() const override;
  ui::EventTargeter* GetEventTargeter() override;
  void ConvertEventToTarget(ui::EventTarget* target,
                            ui::LocatedEvent* event) override;

 private:
  // The WindowManagerApp which owns us.
  WindowManagerApp* app_;

  // The mojo::View that we dispatch to.
  View* view_;

  std::vector<ViewTarget*> children_;

  scoped_ptr<ViewTargeter> targeter_;

  DISALLOW_COPY_AND_ASSIGN(ViewTarget);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_VIEW_TARGET_H_
