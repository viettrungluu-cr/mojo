// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_TEST_UTIL_H_
#define MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_TEST_UTIL_H_

#include <set>

#include "mojo/services/public/cpp/view_manager/lib/view_private.h"
#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/window_manager/view_target.h"

namespace gfx {
class Rect;
}

namespace mojo {

// A wrapper around View so we can instantiate these directly without a
// ViewManager.
class TestView : public View {
 public:
  TestView(int id, const gfx::Rect& rect);
  ~TestView();

  ViewTarget* target() { return target_; }

 private:
  ViewTarget* target_;

  DISALLOW_COPY_AND_ASSIGN(TestView);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_TEST_UTIL_H_
