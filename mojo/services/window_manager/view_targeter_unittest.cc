// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/view_targeter.h"

#include "mojo/services/window_manager/view_event_dispatcher.h"
#include "mojo/services/window_manager/window_manager_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/events/test/test_event_handler.h"

namespace mojo {

class ViewTargeterTest : public testing::Test {
 public:
  ViewTargeterTest() {}
  ~ViewTargeterTest() override {}

  void SetUp() override {
    view_event_dispatcher_.reset(new ViewEventDispatcher());
  }

  void TearDown() override {
    view_event_dispatcher_.reset();
    testing::Test::TearDown();
  }

 protected:
  scoped_ptr<ViewEventDispatcher> view_event_dispatcher_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ViewTargeterTest);
};

TEST_F(ViewTargeterTest, Basic) {
  // The dispatcher will take ownership of the tree root.
  TestView root(1, gfx::Rect(0, 0, 100, 100));
  ViewTarget* root_target = root.target();
  root_target->SetEventTargeter(scoped_ptr<ViewTargeter>(new ViewTargeter()));
  view_event_dispatcher_->SetRootViewTarget(root_target);

  TestView one(2, gfx::Rect(0, 0, 500, 100));
  TestView two(3, gfx::Rect(501, 0, 500, 1000));

  root.AddChild(&one);
  root.AddChild(&two);

  ui::test::TestEventHandler handler;
  one.target()->AddPreTargetHandler(&handler);

  ui::MouseEvent press(ui::ET_MOUSE_PRESSED, gfx::Point(20, 20),
                       gfx::Point(20, 20), ui::EF_NONE, ui::EF_NONE);
  ui::EventDispatchDetails details =
      view_event_dispatcher_->OnEventFromSource(&press);
  ASSERT_FALSE(details.dispatcher_destroyed);

  EXPECT_EQ(1, handler.num_mouse_events());

  one.target()->RemovePreTargetHandler(&handler);
}

}  // namespace mojo
