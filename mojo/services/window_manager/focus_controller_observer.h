// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_FOCUS_CONTROLLER_OBSERVER_H_
#define MOJO_SERVICES_WINDOW_MANAGER_FOCUS_CONTROLLER_OBSERVER_H_

namespace mojo {

class View;

class FocusControllerObserver {
 public:
  // Called when |active| gains focus, or there is no active view
  // (|active| is null in this case.) |old_active| refers to the
  // previous active view or null if there was no previously active
  // view.
  virtual void OnViewActivated(View* gained_active,
                               View* lost_active) = 0;

  // Called when focus moves from |lost_focus| to |gained_focus|.
  virtual void OnViewFocused(View* gained_focus, View* lost_focus) = 0;

  // Called when during view activation the currently active view is
  // selected for activation. This can happen when a view requested for
  // activation cannot be activated because a system modal view is active.
  virtual void OnAttemptToReactivateView(View* request_active,
                                         View* actual_active) {}

 protected:
  virtual ~FocusControllerObserver() {}
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_FOCUS_CONTROLLER_OBSERVER_H_
