// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/basic_focus_rules.h"

#include "base/macros.h"
#include "mojo/services/public/cpp/view_manager/view.h"

namespace mojo {

BasicFocusRules::BasicFocusRules(mojo::View* window_container)
    : window_container_(window_container) {
}

BasicFocusRules::~BasicFocusRules() {}

bool BasicFocusRules::IsToplevelView(mojo::View* view) const {
  return view->parent() == window_container_;
}

bool BasicFocusRules::CanActivateView(mojo::View* view) const {
  // TODO(erg): This needs to check visibility, along with focus, and several
  // other things (see wm::BaseFocusRules).
  return view->parent() == window_container_;
}

bool BasicFocusRules::CanFocusView(mojo::View* view) const {
  return true;
}

mojo::View* BasicFocusRules::GetToplevelView(mojo::View* view) const {
  while (view->parent() != window_container_) {
    view = view->parent();
    // Unparented hierarchy, there is no "top level" window.
    if (!view)
      return nullptr;
  }

  return view;
}

mojo::View* BasicFocusRules::GetActivatableView(mojo::View* view) const {
  return GetToplevelView(view);
}

mojo::View* BasicFocusRules::GetFocusableView(mojo::View* view) const {
  return view;
}

mojo::View* BasicFocusRules::GetNextActivatableView(
    mojo::View* activatable) const {
  const mojo::View::Children& children = activatable->parent()->children();
  for (mojo::View::Children::const_reverse_iterator it = children.rbegin();
       it != children.rend(); ++it) {
    if (*it != activatable)
      return *it;
  }
  return nullptr;
}

}  // namespace mojo
