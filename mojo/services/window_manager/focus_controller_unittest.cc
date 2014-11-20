// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/focus_controller.h"

#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/services/window_manager/basic_focus_rules.h"
#include "mojo/services/window_manager/focus_controller_observer.h"
#include "mojo/services/window_manager/view_event_dispatcher.h"
#include "mojo/services/window_manager/view_targeter.h"
#include "mojo/services/window_manager/window_manager_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/events/test/event_generator.h"
#include "ui/gfx/geometry/rect.h"

namespace mojo {

// Counts the number of events that occur.
class FocusNotificationObserver : public FocusControllerObserver {
 public:
  FocusNotificationObserver()
      : activation_changed_count_(0),
        focus_changed_count_(0),
        reactivation_count_(0),
        reactivation_requested_window_(NULL),
        reactivation_actual_window_(NULL) {}
  ~FocusNotificationObserver() override {}

  void ExpectCounts(int activation_changed_count, int focus_changed_count) {
    EXPECT_EQ(activation_changed_count, activation_changed_count_);
    EXPECT_EQ(focus_changed_count, focus_changed_count_);
  }
  int reactivation_count() const { return reactivation_count_; }
  View* reactivation_requested_window() const {
    return reactivation_requested_window_;
  }
  View* reactivation_actual_window() const {
    return reactivation_actual_window_;
  }

 protected:
  // Overridden from FocusControllerObserver:
  void OnViewActivated(View* gained_active, View* lost_active) override {
    ++activation_changed_count_;
  }

  void OnViewFocused(View* gained_focus, View* lost_focus) override {
    ++focus_changed_count_;
  }

  void OnAttemptToReactivateView(View* request_active,
                                 View* actual_active) override {
    ++reactivation_count_;
    reactivation_requested_window_ = request_active;
    reactivation_actual_window_ = actual_active;
  }

 private:
  int activation_changed_count_;
  int focus_changed_count_;
  int reactivation_count_;
  View* reactivation_requested_window_;
  View* reactivation_actual_window_;

  DISALLOW_COPY_AND_ASSIGN(FocusNotificationObserver);
};

class ScopedFocusNotificationObserver : public FocusNotificationObserver {
 public:
  ScopedFocusNotificationObserver(FocusController* focus_controller)
      : focus_controller_(focus_controller) {
    focus_controller_->AddObserver(this);
  }
  ~ScopedFocusNotificationObserver() override {
    focus_controller_->RemoveObserver(this);
  }

 private:
  mojo::FocusController* focus_controller_;

  DISALLOW_COPY_AND_ASSIGN(ScopedFocusNotificationObserver);
};

// Only responds to events if a message contains |target| as a parameter.
class ScopedFilteringFocusNotificationObserver
    : public FocusNotificationObserver {
 public:
  ScopedFilteringFocusNotificationObserver(FocusController* focus_controller,
                                           View* target)
      : focus_controller_(focus_controller), target_(target) {
    focus_controller_->AddObserver(this);
  }
  ~ScopedFilteringFocusNotificationObserver() override {
    focus_controller_->RemoveObserver(this);
  }

 private:
  // Overridden from FocusControllerObserver:
  void OnViewActivated(View* gained_active, View* lost_active) override {
    if (gained_active == target_ || lost_active == target_)
      FocusNotificationObserver::OnViewActivated(gained_active, lost_active);
  }

  void OnViewFocused(View* gained_focus, View* lost_focus) override {
    if (gained_focus == target_ || lost_focus == target_)
      FocusNotificationObserver::OnViewFocused(gained_focus, lost_focus);
  }

  void OnAttemptToReactivateView(View* request_active,
                                 View* actual_active) override {
    if (request_active == target_ || actual_active == target_) {
      FocusNotificationObserver::OnAttemptToReactivateView(request_active,
                                                           actual_active);
    }
  }

  mojo::FocusController* focus_controller_;
  View* target_;

  DISALLOW_COPY_AND_ASSIGN(ScopedFilteringFocusNotificationObserver);
};

// Used to fake the handling of events in the pre-target phase.
class SimpleEventHandler : public ui::EventHandler {
 public:
  SimpleEventHandler() {}
  ~SimpleEventHandler() override {}

  // Overridden from ui::EventHandler:
  void OnMouseEvent(ui::MouseEvent* event) override {
    event->SetHandled();
  }
  void OnGestureEvent(ui::GestureEvent* event) override {
    event->SetHandled();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(SimpleEventHandler);
};

// BasicFocusRules subclass that allows basic overrides of focus/activation to
// be tested. This is intended more as a test that the override system works at
// all, rather than as an exhaustive set of use cases, those should be covered
// in tests for those FocusRules implementations.
class TestFocusRules : public BasicFocusRules {
 public:
  TestFocusRules(View* root)
      : BasicFocusRules(root), focus_restriction_(NULL) {}

  // Restricts focus and activation to this window and its child hierarchy.
  void set_focus_restriction(View* focus_restriction) {
    focus_restriction_ = focus_restriction;
  }

  // Overridden from BasicFocusRules:
  bool CanActivateView(View* view) const override {
    // Restricting focus to a non-activatable child view means the activatable
    // parent outside the focus restriction is activatable.
    bool can_activate =
        CanFocusOrActivate(view) || view->Contains(focus_restriction_);
    return can_activate ? BasicFocusRules::CanActivateView(view) : false;
  }
  bool CanFocusView(View* view) const override {
    return CanFocusOrActivate(view) ? BasicFocusRules::CanFocusView(view)
                                    : false;
  }
  View* GetActivatableView(View* view) const override {
    return BasicFocusRules::GetActivatableView(
        CanFocusOrActivate(view) ? view : focus_restriction_);
  }
  View* GetFocusableView(View* view) const override {
    return BasicFocusRules::GetFocusableView(
        CanFocusOrActivate(view) ? view : focus_restriction_);
  }
  View* GetNextActivatableView(View* ignore) const override {
    View* next_activatable = BasicFocusRules::GetNextActivatableView(ignore);
    return CanFocusOrActivate(next_activatable)
               ? next_activatable
               : GetActivatableView(focus_restriction_);
  }

 private:
  bool CanFocusOrActivate(View* view) const {
    return !focus_restriction_ || focus_restriction_->Contains(view);
  }

  View* focus_restriction_;

  DISALLOW_COPY_AND_ASSIGN(TestFocusRules);
};

// Common infrastructure shared by all FocusController test types.
class FocusControllerTestBase : public testing::Test {
 protected:
  // Hierarchy used by all tests:
  // root_window
  //       +-- w1
  //       |    +-- w11
  //       |    +-- w12
  //       +-- w2
  //       |    +-- w21
  //       |         +-- w211
  //       +-- w3
  FocusControllerTestBase()
      : root_view_(0, gfx::Rect(0, 0, 800, 600)),
        v1(1, gfx::Rect(0, 0, 50, 50), root_view()),
        v11(11, gfx::Rect(5, 5, 10, 10), &v1),
        v12(12, gfx::Rect(15, 15, 10, 10), &v1),
        v2(2, gfx::Rect(75, 75, 50, 50), root_view()),
        v21(21, gfx::Rect(5, 5, 10, 10), &v2),
        v211(211, gfx::Rect(1, 1, 5, 5), &v21),
        v3(3, gfx::Rect(125, 125, 50, 50), root_view()) {}

  // Overridden from testing::Test:
  void SetUp() override {
    testing::Test::SetUp();

    test_focus_rules_ = new TestFocusRules(root_view());
    focus_controller_.reset(
        new FocusController(scoped_ptr<FocusRules>(test_focus_rules_)));

    ViewTarget* root_target = root_view_.target();
    root_target->SetEventTargeter(scoped_ptr<ViewTargeter>(new ViewTargeter()));
    view_event_dispatcher_.reset(new ViewEventDispatcher());
    view_event_dispatcher_->SetRootViewTarget(root_target);

    GetRootViewTarget()->AddPreTargetHandler(focus_controller_.get());
  }

  void TearDown() override {
    GetRootViewTarget()->RemovePreTargetHandler(focus_controller_.get());
    view_event_dispatcher_.reset();

    test_focus_rules_ = nullptr;  // Owned by FocusController.
    focus_controller_.reset();

    testing::Test::TearDown();
  }

  void FocusView(View* view) { focus_controller_->FocusView(view); }
  View* GetFocusedView() { return focus_controller_->GetFocusedView(); }
  int GetFocusedViewId() {
    View* focused_view = GetFocusedView();
    return focused_view ? focused_view->id() : -1;
  }
  void ActivateView(View* view) { focus_controller_->ActivateView(view); }
  void DeactivateView(View* view) { focus_controller_->DeactivateView(view); }
  View* GetActiveView() { return focus_controller_->GetActiveView(); }
  int GetActiveViewId() {
    View* active_view = GetActiveView();
    return active_view ? active_view->id() : -1;
  }

  View* GetViewById(int id) { return root_view_.GetChildById(id); }

  void ClickLeftButton(View* view) {
    // Get the center bounds of |target| in |root_view_| coordinate space.
    gfx::Point center =
        gfx::Rect(view->bounds().To<gfx::Rect>().size()).CenterPoint();
    ViewTarget::ConvertPointToTarget(ViewTarget::TargetFromView(view),
                                     root_view_.target(), &center);

    ui::MouseEvent button_down(ui::ET_MOUSE_PRESSED, center, center,
                               ui::EF_LEFT_MOUSE_BUTTON, ui::EF_NONE);
    ui::EventDispatchDetails details =
        view_event_dispatcher_->OnEventFromSource(&button_down);
    CHECK(!details.dispatcher_destroyed);

    ui::MouseEvent button_up(ui::ET_MOUSE_RELEASED, center, center,
                             ui::EF_LEFT_MOUSE_BUTTON, ui::EF_NONE);
    details = view_event_dispatcher_->OnEventFromSource(&button_up);
    CHECK(!details.dispatcher_destroyed);
  }

  ViewTarget* GetRootViewTarget() {
    return ViewTarget::TargetFromView(root_view());
  }

  View* root_view() { return &root_view_; }
  TestFocusRules* test_focus_rules() { return test_focus_rules_; }
  FocusController* focus_controller() { return focus_controller_.get(); }

  // Test functions.
  virtual void BasicFocus() = 0;
  virtual void BasicActivation() = 0;
  virtual void FocusEvents() = 0;

 private:
  TestView root_view_;
  scoped_ptr<FocusController> focus_controller_;
  TestFocusRules* test_focus_rules_;
  // TODO(erg): The aura version of this class also keeps track of WMState. Do
  // we need something analogous here?

  scoped_ptr<ViewEventDispatcher> view_event_dispatcher_;

  TestView v1;
  TestView v11;
  TestView v12;
  TestView v2;
  TestView v21;
  TestView v211;
  TestView v3;

  DISALLOW_COPY_AND_ASSIGN(FocusControllerTestBase);
};

// Test base for tests where focus is directly set to a target window.
class FocusControllerDirectTestBase : public FocusControllerTestBase {
 protected:
  FocusControllerDirectTestBase() {}

  // Different test types shift focus in different ways.
  virtual void FocusViewDirect(View* view) = 0;
  virtual void ActivateViewDirect(View* view) = 0;
  virtual void DeactivateViewDirect(View* view) = 0;

  // Input events do not change focus if the window can not be focused.
  virtual bool IsInputEvent() = 0;

  void FocusViewById(int id) {
    View* view = root_view()->GetChildById(id);
    DCHECK(view);
    FocusViewDirect(view);
  }
  void ActivateViewById(int id) {
    View* view = root_view()->GetChildById(id);
    DCHECK(view);
    ActivateViewDirect(view);
  }

  // Overridden from FocusControllerTestBase:
  void BasicFocus() override {
    EXPECT_EQ(nullptr, GetFocusedView());
    FocusViewById(1);
    EXPECT_EQ(1, GetFocusedViewId());
    FocusViewById(2);
    EXPECT_EQ(2, GetFocusedViewId());
  }
  void BasicActivation() override {
    EXPECT_EQ(nullptr, GetActiveView());
    ActivateViewById(1);
    EXPECT_EQ(1, GetActiveViewId());
    ActivateViewById(2);
    EXPECT_EQ(2, GetActiveViewId());
    // Verify that attempting to deactivate NULL does not crash and does not
    // change activation.
    DeactivateView(nullptr);
    EXPECT_EQ(2, GetActiveViewId());
    DeactivateView(GetActiveView());
    EXPECT_EQ(1, GetActiveViewId());
  }
  void FocusEvents() override {
    ScopedFocusNotificationObserver root_observer(focus_controller());
    ScopedFilteringFocusNotificationObserver observer1(focus_controller(),
                                                       GetViewById(1));
    ScopedFilteringFocusNotificationObserver observer2(focus_controller(),
                                                       GetViewById(2));

    root_observer.ExpectCounts(0, 0);
    observer1.ExpectCounts(0, 0);
    observer2.ExpectCounts(0, 0);

    FocusViewById(1);
    root_observer.ExpectCounts(1, 1);
    observer1.ExpectCounts(1, 1);
    observer2.ExpectCounts(0, 0);

    FocusViewById(2);
    root_observer.ExpectCounts(2, 2);
    observer1.ExpectCounts(2, 2);
    observer2.ExpectCounts(1, 1);
  }

  // TODO(erg): There are a whole bunch other tests here. Port them.

 private:
  DISALLOW_COPY_AND_ASSIGN(FocusControllerDirectTestBase);
};

// Focus and Activation changes via the FocusController API.
class FocusControllerApiTest : public FocusControllerDirectTestBase {
 public:
  FocusControllerApiTest() {}

 private:
  // Overridden from FocusControllerTestBase:
  void FocusViewDirect(View* view) override { FocusView(view); }
  void ActivateViewDirect(View* view) override { ActivateView(view); }
  void DeactivateViewDirect(View* view) override { DeactivateView(view); }
  bool IsInputEvent() override { return false; }

  DISALLOW_COPY_AND_ASSIGN(FocusControllerApiTest);
};

// Focus and Activation changes via input events.
class FocusControllerMouseEventTest : public FocusControllerDirectTestBase {
 public:
  FocusControllerMouseEventTest() {}

  // Tests that a handled mouse event does not trigger a window activation.
  void IgnoreHandledEvent() {
    EXPECT_EQ(NULL, GetActiveView());
    View* v1 = root_view()->GetChildById(1);
    SimpleEventHandler handler;
    GetRootViewTarget()->PrependPreTargetHandler(&handler);
    ClickLeftButton(v1);
    EXPECT_EQ(NULL, GetActiveView());
    // TODO(erg): Add gesture testing when we get gestures working.
    GetRootViewTarget()->RemovePreTargetHandler(&handler);
    ClickLeftButton(v1);
    EXPECT_EQ(1, GetActiveViewId());
  }

 private:
  // Overridden from FocusControllerTestBase:
  void FocusViewDirect(View* view) override { ClickLeftButton(view); }
  void ActivateViewDirect(View* view) override { ClickLeftButton(view); }
  void DeactivateViewDirect(View* view) override {
    View* next_activatable = test_focus_rules()->GetNextActivatableView(view);
    ClickLeftButton(next_activatable);
  }
  bool IsInputEvent() override { return true; }

  DISALLOW_COPY_AND_ASSIGN(FocusControllerMouseEventTest);
};

#define FOCUS_CONTROLLER_TEST(TESTCLASS, TESTNAME) \
  TEST_F(TESTCLASS, TESTNAME) { TESTNAME(); }

// Runs direct focus change tests (input events and API calls).
//
// TODO(erg): Enable gesture events in the future.
#define DIRECT_FOCUS_CHANGE_TESTS(TESTNAME)               \
  FOCUS_CONTROLLER_TEST(FocusControllerApiTest, TESTNAME) \
  FOCUS_CONTROLLER_TEST(FocusControllerMouseEventTest, TESTNAME)

DIRECT_FOCUS_CHANGE_TESTS(BasicFocus);
DIRECT_FOCUS_CHANGE_TESTS(BasicActivation);
DIRECT_FOCUS_CHANGE_TESTS(FocusEvents);

// TODO(erg): Also port IMPLICIT_FOCUS_CHANGE_TARGET_TESTS / ALL_FOCUS_TESTS
// here, and replace the above direct testing list.

// If a mouse event was handled, it should not activate a window.
FOCUS_CONTROLLER_TEST(FocusControllerMouseEventTest, IgnoreHandledEvent);

}  // namespace mojo
