// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERIVCES_WINDOW_MANAGER_BASIC_FOCUS_RULES_H_
#define MOJO_SERIVCES_WINDOW_MANAGER_BASIC_FOCUS_RULES_H_

#include "mojo/services/window_manager/focus_rules.h"

namespace mojo {

class View;

// The focusing rules used inside a window manager.
//
// This is intended to be a user supplyable, subclassable component passed to
// WindowManagerApp, allowing for the creation of other window managers.
//
// TODO(erg): This was a straight move of the minimal focus rules from when
// WindowManagerApp directly used the wm::FocusController. This implementation
// is very incomplete. crbug.com/431047
class BasicFocusRules : public FocusRules {
 public:
  BasicFocusRules(mojo::View* window_container);
  ~BasicFocusRules() override;

 protected:
  // Overridden from mojo::FocusRules:
  bool IsToplevelView(mojo::View* view) const override;
  bool CanActivateView(mojo::View* view) const override;
  bool CanFocusView(mojo::View* view) const override;
  mojo::View* GetToplevelView(mojo::View* view) const override;
  mojo::View* GetActivatableView(mojo::View* view) const override;
  mojo::View* GetFocusableView(mojo::View* view) const override;
  mojo::View* GetNextActivatableView(mojo::View* activatable) const override;

 private:
  mojo::View* window_container_;

  DISALLOW_COPY_AND_ASSIGN(BasicFocusRules);
};

}  // namespace mojo

#endif  // MOJO_SERIVCES_WINDOW_MANAGER_BASIC_FOCUS_RULES_H_
