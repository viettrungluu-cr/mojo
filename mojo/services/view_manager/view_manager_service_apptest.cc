// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/application_test_base.h"
#include "mojo/services/public/interfaces/view_manager/view_manager.mojom.h"
#include "mojo/services/public/interfaces/window_manager/window_manager.mojom.h"
#include "mojo/services/public/interfaces/window_manager/window_manager_internal.mojom.h"
#include "mojo/services/view_manager/ids.h"
#include "mojo/services/view_manager/test_change_tracker.h"

namespace mojo {
namespace service {

// Creates an id used for transport from the specified parameters.
Id BuildViewId(ConnectionSpecificId connection_id,
               ConnectionSpecificId view_id) {
  return (connection_id << 16) | view_id;
}

// Callback function from ViewManagerService functions. ------------------------

void BoolResultCallback(base::RunLoop* run_loop,
                        bool* result_cache,
                        bool result) {
  *result_cache = result;
  run_loop->Quit();
}

void ErrorCodeResultCallback(base::RunLoop* run_loop,
                             ErrorCode* result_cache,
                             ErrorCode result) {
  *result_cache = result;
  run_loop->Quit();
}

void ViewTreeResultCallback(base::RunLoop* run_loop,
                            std::vector<TestView>* views,
                            Array<ViewDataPtr> results) {
  ViewDatasToTestViews(results, views);
  run_loop->Quit();
}

// -----------------------------------------------------------------------------

// The following functions call through to the supplied ViewManagerService. They
// block until call completes and return the result.
bool CreateView(ViewManagerService* vm, Id view_id) {
  ErrorCode result = ERROR_CODE_NONE;
  base::RunLoop run_loop;
  vm->CreateView(view_id,
                 base::Bind(&ErrorCodeResultCallback, &run_loop, &result));
  run_loop.Run();
  return result == ERROR_CODE_NONE;
}

bool Embed(ViewManagerService* vm, Id root_id) {
  bool result = false;
  base::RunLoop run_loop;
  {
    ServiceProviderPtr sp;
    vm->Embed("mojo:view_manager_service_apptests", root_id,
              MakeRequest<ServiceProvider>(sp.PassMessagePipe()),
              base::Bind(&BoolResultCallback, &run_loop, &result));
  }
  run_loop.Run();
  return result;
}

ErrorCode CreateViewWithErrorCode(ViewManagerService* vm, Id view_id) {
  ErrorCode result = ERROR_CODE_NONE;
  base::RunLoop run_loop;
  vm->CreateView(view_id,
                 base::Bind(&ErrorCodeResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

bool AddView(ViewManagerService* vm, Id parent, Id child) {
  bool result = false;
  base::RunLoop run_loop;
  vm->AddView(parent, child,
              base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

bool RemoveViewFromParent(ViewManagerService* vm, Id view_id) {
  bool result = false;
  base::RunLoop run_loop;
  vm->RemoveViewFromParent(view_id,
                           base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

bool ReorderView(ViewManagerService* vm,
                 Id view_id,
                 Id relative_view_id,
                 OrderDirection direction) {
  bool result = false;
  base::RunLoop run_loop;
  vm->ReorderView(view_id, relative_view_id, direction,
                  base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

void GetViewTree(ViewManagerService* vm,
                 Id view_id,
                 std::vector<TestView>* views) {
  base::RunLoop run_loop;
  vm->GetViewTree(view_id,
                  base::Bind(&ViewTreeResultCallback, &run_loop, views));
  run_loop.Run();
}

bool DeleteView(ViewManagerService* vm, Id view_id) {
  base::RunLoop run_loop;
  bool result = false;
  vm->DeleteView(view_id, base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

bool SetViewBounds(ViewManagerService* vm,
                   Id view_id,
                   int x,
                   int y,
                   int w,
                   int h) {
  base::RunLoop run_loop;
  bool result = false;
  RectPtr rect(Rect::New());
  rect->x = x;
  rect->y = y;
  rect->width = w;
  rect->height = h;
  vm->SetViewBounds(view_id, rect.Pass(),
                    base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

bool SetViewVisibility(ViewManagerService* vm, Id view_id, bool visible) {
  base::RunLoop run_loop;
  bool result = false;
  vm->SetViewVisibility(view_id, visible,
                        base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

bool SetViewProperty(ViewManagerService* vm,
                     Id view_id,
                     const std::string& name,
                     const std::vector<uint8_t>* data) {
  base::RunLoop run_loop;
  bool result = false;
  Array<uint8_t> mojo_data;
  if (data)
    mojo_data = Array<uint8_t>::From(*data);
  vm->SetViewProperty(view_id, name, mojo_data.Pass(),
                      base::Bind(&BoolResultCallback, &run_loop, &result));
  run_loop.Run();
  return result;
}

// -----------------------------------------------------------------------------

// A ViewManagerClient implementation that logs all changes to a tracker.
class ViewManagerClientImpl : public InterfaceImpl<ViewManagerClient>,
                              public TestChangeTracker::Delegate {
 public:
  ViewManagerClientImpl() : got_embed_(false) { tracker_.set_delegate(this); }

  TestChangeTracker* tracker() { return &tracker_; }

  // Runs a nested MessageLoop until |count| changes (calls to
  // ViewManagerClient functions) have been received.
  void WaitForChangeCount(size_t count) {
    if (count == tracker_.changes()->size())
      return;

    ASSERT_TRUE(wait_state_.get() == nullptr);
    wait_state_.reset(new WaitState);
    wait_state_->change_count = count;
    wait_state_->run_loop.Run();
    wait_state_.reset();
  }

  // Runs a nested MessageLoop until OnEmbed() has been encountered.
  void WaitForOnEmbed() {
    if (got_embed_)
      return;
    embed_run_loop_.reset(new base::RunLoop);
    embed_run_loop_->Run();
    embed_run_loop_.reset();
  }

 private:
  // Used when running a nested MessageLoop.
  struct WaitState {
    WaitState() : change_count(0) {}

    // Number of changes waiting for.
    size_t change_count;
    base::RunLoop run_loop;
  };

  // TestChangeTracker::Delegate:
  void OnChangeAdded() override {
    if (wait_state_.get() &&
        wait_state_->change_count == tracker_.changes()->size()) {
      wait_state_->run_loop.Quit();
    }
  }

  // ViewManagerClient:
  void OnEmbed(ConnectionSpecificId connection_id,
               const String& creator_url,
               ViewDataPtr root,
               InterfaceRequest<ServiceProvider> services,
               ScopedMessagePipeHandle window_manager_pipe) override {
    tracker()->OnEmbed(connection_id, creator_url, root.Pass());
    got_embed_ = true;
    if (embed_run_loop_)
      embed_run_loop_->Quit();
  }
  void OnEmbeddedAppDisconnected(Id view_id) override {
    tracker()->OnEmbeddedAppDisconnected(view_id);
  }
  void OnViewBoundsChanged(Id view_id,
                           RectPtr old_bounds,
                           RectPtr new_bounds) override {
    tracker()->OnViewBoundsChanged(view_id, old_bounds.Pass(),
                                   new_bounds.Pass());
  }
  void OnViewHierarchyChanged(Id view,
                              Id new_parent,
                              Id old_parent,
                              Array<ViewDataPtr> views) override {
    tracker()->OnViewHierarchyChanged(view, new_parent, old_parent,
                                      views.Pass());
  }
  void OnViewReordered(Id view_id,
                       Id relative_view_id,
                       OrderDirection direction) override {
    tracker()->OnViewReordered(view_id, relative_view_id, direction);
  }
  void OnViewDeleted(Id view) override { tracker()->OnViewDeleted(view); }
  void OnViewVisibilityChanged(uint32_t view, bool visible) override {
    tracker()->OnViewVisibilityChanged(view, visible);
  }
  void OnViewDrawnStateChanged(uint32_t view, bool drawn) override {
    tracker()->OnViewDrawnStateChanged(view, drawn);
  }
  void OnViewInputEvent(Id view_id,
                        EventPtr event,
                        const Callback<void()>& callback) override {
    tracker()->OnViewInputEvent(view_id, event.Pass());
  }
  void OnViewSharedPropertyChanged(uint32_t view,
                                   const String& name,
                                   Array<uint8_t> new_data) override {
    tracker_.OnViewSharedPropertyChanged(view, name, new_data.Pass());
  }

  TestChangeTracker tracker_;

  // Whether OnEmbed() has been encountered.
  bool got_embed_;

  // If non-null we're waiting for OnEmbed() using this RunLoop.
  scoped_ptr<base::RunLoop> embed_run_loop_;

  // If non-null we're waiting for a certain number of change notifications to
  // be encountered.
  scoped_ptr<WaitState> wait_state_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerClientImpl);
};

// -----------------------------------------------------------------------------

// InterfaceFactory for vending ViewManagerClientImpls.
class ViewManagerClientFactory : public InterfaceFactory<ViewManagerClient> {
 public:
  ViewManagerClientFactory() {}
  ~ViewManagerClientFactory() override {}

  // Runs a nested MessageLoop until a new instance has been created.
  scoped_ptr<ViewManagerClientImpl> WaitForInstance() {
    if (!client_impl_.get()) {
      DCHECK(!run_loop_.get());
      run_loop_.reset(new base::RunLoop);
      run_loop_->Run();
      run_loop_.reset();
    }
    return client_impl_.Pass();
  }

 private:
  // InterfaceFactory<ViewManagerClient>:
  void Create(ApplicationConnection* connection,
              InterfaceRequest<ViewManagerClient> request) override {
    client_impl_.reset(new ViewManagerClientImpl);
    WeakBindToRequest(client_impl_.get(), &request);
    if (run_loop_.get())
      run_loop_->Quit();
  }

  scoped_ptr<ViewManagerClientImpl> client_impl_;
  scoped_ptr<base::RunLoop> run_loop_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerClientFactory);
};

class ViewManagerServiceAppTest : public test::ApplicationTestBase,
                                  public ApplicationDelegate {
 public:
  ViewManagerServiceAppTest() : ApplicationTestBase(Array<String>()) {}
  ~ViewManagerServiceAppTest() override {}

 protected:
  // Returns the changes from the various connections.
  std::vector<Change>* changes1() { return vm_client1_.tracker()->changes(); }
  std::vector<Change>* changes2() { return vm_client2_->tracker()->changes(); }
  std::vector<Change>* changes3() { return vm_client3_->tracker()->changes(); }

  // Various connections. |vm1()|, being the first connection, has special
  // permissions (it's treated as the window manager).
  ViewManagerService* vm1() { return vm1_.get(); }
  ViewManagerService* vm2() { return vm_client2_->client(); }
  ViewManagerService* vm3() { return vm_client3_->client(); }

  void EstablishSecondConnectionWithRoot(Id root_id) {
    ASSERT_TRUE(vm_client2_.get() == nullptr);
    vm_client2_ = EstablishConnectionViaEmbed(vm1(), root_id);
    ASSERT_TRUE(vm_client2_.get() != nullptr);
  }

  void EstablishSecondConnection(bool create_initial_view) {
    if (create_initial_view)
      ASSERT_TRUE(CreateView(vm1_.get(), BuildViewId(1, 1)));
    ASSERT_NO_FATAL_FAILURE(
        EstablishSecondConnectionWithRoot(BuildViewId(1, 1)));

    if (create_initial_view)
      EXPECT_EQ("[view=1,1 parent=null]", ChangeViewDescription(*changes2()));
  }

  void EstablishThirdConnection(ViewManagerService* owner, Id root_id) {
    ASSERT_TRUE(vm_client3_.get() == nullptr);
    vm_client3_ = EstablishConnectionViaEmbed(owner, root_id);
    ASSERT_TRUE(vm_client3_.get() != nullptr);
  }

  // Establishes a new connection by way of Embed() on the specified
  // ViewManagerService.
  scoped_ptr<ViewManagerClientImpl> EstablishConnectionViaEmbed(
      ViewManagerService* owner,
      Id root_id) {
    if (!Embed(owner, root_id)) {
      ADD_FAILURE() << "Embed() failed";
      return nullptr;
    }
    scoped_ptr<ViewManagerClientImpl> client =
        client_factory_.WaitForInstance();
    if (!client.get()) {
      ADD_FAILURE() << "WaitForInstance failed";
      return nullptr;
    }
    client->WaitForOnEmbed();

    const std::string expected_creator =
        owner == vm1() ? "mojo:window_manager"
                       : "mojo:view_manager_service_apptests";
    EXPECT_EQ("OnEmbed creator=" + expected_creator,
              SingleChangeToDescription(*client->tracker()->changes()));
    return client.Pass();
  }

  // ApplicationTestBase:
  ApplicationDelegate* GetApplicationDelegate() override { return this; }
  void SetUp() override {
    ApplicationTestBase::SetUp();
    ApplicationConnection* vm_connection =
        application_impl()->ConnectToApplication("mojo:view_manager");
    vm_connection->ConnectToService(&vm1_);
    vm1_.set_client(&vm_client1_);
    vm_connection->ConnectToService(&wm_internal_);
    vm1_.WaitForIncomingMethodCall();
    ASSERT_EQ(1u, changes1()->size());
    EXPECT_EQ(CHANGE_TYPE_EMBED, (*changes1())[0].type);
    // All these tests assume 1 for the client id. The only real assertion here
    // is the client id is not zero, but adding this as rest of code here
    // assumes 1.
    ASSERT_EQ(1, (*changes1())[0].connection_id);
    changes1()->clear();
  }

  // ApplicationDelegate implementation.
  void Initialize(ApplicationImpl* app) override {}
  bool ConfigureIncomingConnection(ApplicationConnection* connection) override {
    connection->AddService(&client_factory_);
    return true;
  }

  WindowManagerInternalClientPtr wm_internal_;
  ViewManagerClientImpl vm_client1_;
  scoped_ptr<ViewManagerClientImpl> vm_client2_;
  scoped_ptr<ViewManagerClientImpl> vm_client3_;

 private:
  ViewManagerServicePtr vm1_;
  ViewManagerClientFactory client_factory_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(ViewManagerServiceAppTest);
};

// Verifies two clients/connections get different ids.
TEST_F(ViewManagerServiceAppTest, TwoClientsGetDifferentConnectionIds) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  // It isn't strictly necessary that the second connection gets 2, but these
  // tests are written assuming that is the case. The key thing is the
  // connection ids of |connection_| and |connection2_| differ.
  ASSERT_EQ(1u, changes2()->size());
  ASSERT_EQ(2, (*changes2())[0].connection_id);
}

// Verifies when Embed() is invoked any child views are removed.
TEST_F(ViewManagerServiceAppTest, ViewsRemovedWhenEmbedding) {
  // Two views 1 and 2. 2 is parented to 1.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 1), BuildViewId(1, 2)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(false));
  EXPECT_EQ("[view=1,1 parent=null]", ChangeViewDescription(*changes2()));

  // Embed() removed view 2.
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 2), &views);
    EXPECT_EQ("view=1,2 parent=null", SingleViewDescription(views));
  }

  // vm2 should not see view 2.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(1, 1), &views);
    EXPECT_EQ("view=1,1 parent=null", SingleViewDescription(views));
  }
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(1, 2), &views);
    EXPECT_TRUE(views.empty());
  }

  // Views 3 and 4 in connection 2.
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 3)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 4)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(2, 3), BuildViewId(2, 4)));

  // Connection 3 rooted at 2.
  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm2(), BuildViewId(2, 3)));

  // View 4 should no longer have a parent.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(2, 3), &views);
    EXPECT_EQ("view=2,3 parent=null", SingleViewDescription(views));

    views.clear();
    GetViewTree(vm2(), BuildViewId(2, 4), &views);
    EXPECT_EQ("view=2,4 parent=null", SingleViewDescription(views));
  }

  // And view 4 should not be visible to connection 3.
  {
    std::vector<TestView> views;
    GetViewTree(vm3(), BuildViewId(2, 3), &views);
    EXPECT_EQ("view=2,3 parent=null", SingleViewDescription(views));
  }
}

// Verifies once Embed() has been invoked the parent connection can't see any
// children.
TEST_F(ViewManagerServiceAppTest, CantAccessChildrenOfEmbeddedView) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));

  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm2(), BuildViewId(2, 2)));

  ASSERT_TRUE(CreateView(vm3(), BuildViewId(3, 3)));
  ASSERT_TRUE(AddView(vm3(), BuildViewId(2, 2), BuildViewId(3, 3)));

  // Even though 3 is a child of 2 connection 2 can't see 3 as it's from a
  // different connection.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(2, 2), &views);
    EXPECT_EQ("view=2,2 parent=1,1", SingleViewDescription(views));
  }

  // Connection 2 shouldn't be able to get view 3 at all.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(3, 3), &views);
    EXPECT_TRUE(views.empty());
  }

  // Connection 1 should be able to see it all (its the root).
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 1), &views);
    ASSERT_EQ(3u, views.size());
    EXPECT_EQ("view=1,1 parent=null", views[0].ToString());
    EXPECT_EQ("view=2,2 parent=1,1", views[1].ToString());
    EXPECT_EQ("view=3,3 parent=2,2", views[2].ToString());
  }
}

// Verifies once Embed() has been invoked the parent can't mutate the children.
TEST_F(ViewManagerServiceAppTest, CantModifyChildrenOfEmbeddedView) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));

  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm2(), BuildViewId(2, 2)));

  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 3)));
  // Connection 2 shouldn't be able to add anything to the view anymore.
  ASSERT_FALSE(AddView(vm2(), BuildViewId(2, 2), BuildViewId(2, 3)));

  // Create view 3 in connection 3 and add it to view 3.
  ASSERT_TRUE(CreateView(vm3(), BuildViewId(3, 3)));
  ASSERT_TRUE(AddView(vm3(), BuildViewId(2, 2), BuildViewId(3, 3)));

  // Connection 2 shouldn't be able to remove view 3.
  ASSERT_FALSE(RemoveViewFromParent(vm2(), BuildViewId(3, 3)));
}

// Verifies client gets a valid id.
TEST_F(ViewManagerServiceAppTest, CreateView) {
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  EXPECT_TRUE(changes1()->empty());

  // Can't create a view with the same id.
  ASSERT_EQ(ERROR_CODE_VALUE_IN_USE,
            CreateViewWithErrorCode(vm1(), BuildViewId(1, 1)));
  EXPECT_TRUE(changes1()->empty());

  // Can't create a view with a bogus connection id.
  EXPECT_EQ(ERROR_CODE_ILLEGAL_ARGUMENT,
            CreateViewWithErrorCode(vm1(), BuildViewId(2, 1)));
  EXPECT_TRUE(changes1()->empty());
}

// Verifies AddView fails when view is already in position.
TEST_F(ViewManagerServiceAppTest, AddViewWithNoChange) {
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 3)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  // Make 3 a child of 2.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 2), BuildViewId(1, 3)));

  // Try again, this should fail.
  EXPECT_FALSE(AddView(vm1(), BuildViewId(1, 2), BuildViewId(1, 3)));
}

// Verifies AddView fails when view is already in position.
TEST_F(ViewManagerServiceAppTest, AddAncestorFails) {
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 3)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  // Make 3 a child of 2.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 2), BuildViewId(1, 3)));

  // Try to make 2 a child of 3, this should fail since 2 is an ancestor of 3.
  EXPECT_FALSE(AddView(vm1(), BuildViewId(1, 3), BuildViewId(1, 2)));
}

// Verifies adding to root sends right notifications.
TEST_F(ViewManagerServiceAppTest, AddToRoot) {
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 21)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 3)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  changes2()->clear();

  // Make 3 a child of 21.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 21), BuildViewId(1, 3)));

  // Make 21 a child of 1.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 1), BuildViewId(1, 21)));

  // Connection 2 should not be told anything (because the view is from a
  // different connection). Create a view to ensure we got a response from
  // the server.
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 100)));
  EXPECT_TRUE(changes2()->empty());
}

// Verifies HierarchyChanged is correctly sent for various adds/removes.
TEST_F(ViewManagerServiceAppTest, ViewHierarchyChangedViews) {
  // 1,2->1,11.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 11)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 2), BuildViewId(1, 11)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  // 1,1->1,2->1,11
  {
    // Client 2 should not get anything (1,2 is from another connection).
    changes2()->clear();
    ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 1), BuildViewId(1, 2)));
    ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 100)));
    EXPECT_TRUE(changes2()->empty());
  }

  // 0,1->1,1->1,2->1,11.
  {
    // Client 2 is now connected to the root, so it should have gotten a drawn
    // notification.
    ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
    vm_client2_->WaitForChangeCount(1u);
    EXPECT_EQ("DrawnStateChanged view=1,1 drawn=true",
              SingleChangeToDescription(*changes2()));
  }

  // 1,1->1,2->1,11.
  {
    // Client 2 is no longer connected to the root, should get drawn state
    // changed.
    changes2()->clear();
    ASSERT_TRUE(RemoveViewFromParent(vm1(), BuildViewId(1, 1)));
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,1 drawn=false",
              SingleChangeToDescription(*changes2()));
  }

  // 1,1->1,2->1,11->1,111.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 111)));
  {
    changes2()->clear();
    ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 11), BuildViewId(1, 111)));
    ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 103)));
    EXPECT_TRUE(changes2()->empty());
  }

  // 0,1->1,1->1,2->1,11->1,111
  {
    changes2()->clear();
    ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,1 drawn=true",
              SingleChangeToDescription(*changes2()));
  }
}

TEST_F(ViewManagerServiceAppTest, ViewHierarchyChangedAddingKnownToUnknown) {
  // Create the following structure: root -> 1 -> 11 and 2->21 (2 has no
  // parent).
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 11)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 21)));

  // Set up the hierarchy.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 11)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(2, 2), BuildViewId(2, 21)));

  // Remove 11, should result in a hierarchy change for the root.
  {
    changes1()->clear();
    ASSERT_TRUE(RemoveViewFromParent(vm2(), BuildViewId(2, 11)));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("HierarchyChanged view=2,11 new_parent=null old_parent=1,1",
              SingleChangeToDescription(*changes1()));
  }

  // Add 2 to 1.
  {
    changes1()->clear();
    ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("HierarchyChanged view=2,2 new_parent=1,1 old_parent=null",
              SingleChangeToDescription(*changes1()));
    EXPECT_EQ(
        "[view=2,2 parent=1,1],"
        "[view=2,21 parent=2,2]",
        ChangeViewDescription(*changes1()));
  }
}

TEST_F(ViewManagerServiceAppTest, ReorderView) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  Id view1_id = BuildViewId(2, 1);
  Id view2_id = BuildViewId(2, 2);
  Id view3_id = BuildViewId(2, 3);
  Id view4_id = BuildViewId(1, 4);  // Peer to 1,1
  Id view5_id = BuildViewId(1, 5);  // Peer to 1,1
  Id view6_id = BuildViewId(2, 6);  // Child of 1,2.
  Id view7_id = BuildViewId(2, 7);  // Unparented.
  Id view8_id = BuildViewId(2, 8);  // Unparented.
  ASSERT_TRUE(CreateView(vm2(), view1_id));
  ASSERT_TRUE(CreateView(vm2(), view2_id));
  ASSERT_TRUE(CreateView(vm2(), view3_id));
  ASSERT_TRUE(CreateView(vm1(), view4_id));
  ASSERT_TRUE(CreateView(vm1(), view5_id));
  ASSERT_TRUE(CreateView(vm2(), view6_id));
  ASSERT_TRUE(CreateView(vm2(), view7_id));
  ASSERT_TRUE(CreateView(vm2(), view8_id));
  ASSERT_TRUE(AddView(vm2(), view1_id, view2_id));
  ASSERT_TRUE(AddView(vm2(), view2_id, view6_id));
  ASSERT_TRUE(AddView(vm2(), view1_id, view3_id));
  ASSERT_TRUE(AddView(vm1(), ViewIdToTransportId(RootViewId()), view4_id));
  ASSERT_TRUE(AddView(vm1(), ViewIdToTransportId(RootViewId()), view5_id));
  ASSERT_TRUE(AddView(vm1(), ViewIdToTransportId(RootViewId()), view1_id));

  {
    changes1()->clear();
    ASSERT_TRUE(ReorderView(vm2(), view2_id, view3_id, ORDER_DIRECTION_ABOVE));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("Reordered view=2,2 relative=2,3 direction=above",
              SingleChangeToDescription(*changes1()));
  }

  {
    changes1()->clear();
    ASSERT_TRUE(ReorderView(vm2(), view2_id, view3_id, ORDER_DIRECTION_BELOW));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("Reordered view=2,2 relative=2,3 direction=below",
              SingleChangeToDescription(*changes1()));
  }

  // view2 is already below view3.
  EXPECT_FALSE(ReorderView(vm2(), view2_id, view3_id, ORDER_DIRECTION_BELOW));

  // view4 & 5 are unknown to connection2_.
  EXPECT_FALSE(ReorderView(vm2(), view4_id, view5_id, ORDER_DIRECTION_ABOVE));

  // view6 & view3 have different parents.
  EXPECT_FALSE(ReorderView(vm1(), view3_id, view6_id, ORDER_DIRECTION_ABOVE));

  // Non-existent view-ids
  EXPECT_FALSE(ReorderView(vm1(), BuildViewId(1, 27), BuildViewId(1, 28),
                           ORDER_DIRECTION_ABOVE));

  // view7 & view8 are un-parented.
  EXPECT_FALSE(ReorderView(vm1(), view7_id, view8_id, ORDER_DIRECTION_ABOVE));
}

// Verifies DeleteView works.
TEST_F(ViewManagerServiceAppTest, DeleteView) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));

  // Make 2 a child of 1.
  {
    changes1()->clear();
    ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));
    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("HierarchyChanged view=2,2 new_parent=1,1 old_parent=null",
              SingleChangeToDescription(*changes1()));
  }

  // Delete 2.
  {
    changes1()->clear();
    changes2()->clear();
    ASSERT_TRUE(DeleteView(vm2(), BuildViewId(2, 2)));
    EXPECT_TRUE(changes2()->empty());

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("ViewDeleted view=2,2", SingleChangeToDescription(*changes1()));
  }
}

// Verifies DeleteView isn't allowed from a separate connection.
TEST_F(ViewManagerServiceAppTest, DeleteViewFromAnotherConnectionDisallowed) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  EXPECT_FALSE(DeleteView(vm2(), BuildViewId(1, 1)));
}

// Verifies if a view was deleted and then reused that other clients are
// properly notified.
TEST_F(ViewManagerServiceAppTest, ReuseDeletedViewId) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));

  // Add 2 to 1.
  {
    changes1()->clear();
    ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("HierarchyChanged view=2,2 new_parent=1,1 old_parent=null",
              SingleChangeToDescription(*changes1()));
    EXPECT_EQ("[view=2,2 parent=1,1]", ChangeViewDescription(*changes1()));
  }

  // Delete 2.
  {
    changes1()->clear();
    ASSERT_TRUE(DeleteView(vm2(), BuildViewId(2, 2)));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("ViewDeleted view=2,2", SingleChangeToDescription(*changes1()));
  }

  // Create 2 again, and add it back to 1. Should get the same notification.
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  {
    changes1()->clear();
    ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));

    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("HierarchyChanged view=2,2 new_parent=1,1 old_parent=null",
              SingleChangeToDescription(*changes1()));
    EXPECT_EQ("[view=2,2 parent=1,1]", ChangeViewDescription(*changes1()));
  }
}

// Assertions for GetViewTree.
TEST_F(ViewManagerServiceAppTest, GetViewTree) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));

  // Create 11 in first connection and make it a child of 1.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 11)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 1), BuildViewId(1, 11)));

  // Create two views in second connection, 2 and 3, both children of 1.
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 3)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 3)));

  // Verifies GetViewTree() on the root. The root connection sees all.
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(0, 1), &views);
    ASSERT_EQ(5u, views.size());
    EXPECT_EQ("view=0,1 parent=null", views[0].ToString());
    EXPECT_EQ("view=1,1 parent=0,1", views[1].ToString());
    EXPECT_EQ("view=1,11 parent=1,1", views[2].ToString());
    EXPECT_EQ("view=2,2 parent=1,1", views[3].ToString());
    EXPECT_EQ("view=2,3 parent=1,1", views[4].ToString());
  }

  // Verifies GetViewTree() on the view 1,1 from vm2(). vm2() sees 1,1 as 1,1
  // is vm2()'s root and vm2() sees all the views it created.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(1, 1), &views);
    ASSERT_EQ(3u, views.size());
    EXPECT_EQ("view=1,1 parent=null", views[0].ToString());
    EXPECT_EQ("view=2,2 parent=1,1", views[1].ToString());
    EXPECT_EQ("view=2,3 parent=1,1", views[2].ToString());
  }

  // Connection 2 shouldn't be able to get the root tree.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(0, 1), &views);
    ASSERT_EQ(0u, views.size());
  }
}

TEST_F(ViewManagerServiceAppTest, SetViewBounds) {
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(false));

  changes2()->clear();
  ASSERT_TRUE(SetViewBounds(vm1(), BuildViewId(1, 1), 0, 0, 100, 100));

  vm_client2_->WaitForChangeCount(1);
  EXPECT_EQ("BoundsChanged view=1,1 old_bounds=0,0 0x0 new_bounds=0,0 100x100",
            SingleChangeToDescription(*changes2()));

  // Should not be possible to change the bounds of a view created by another
  // connection.
  ASSERT_FALSE(SetViewBounds(vm2(), BuildViewId(1, 1), 0, 0, 0, 0));
}

// Verify AddView fails when trying to manipulate views in other roots.
TEST_F(ViewManagerServiceAppTest, CantMoveViewsFromOtherRoot) {
  // Create 1 and 2 in the first connection.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(false));

  // Try to move 2 to be a child of 1 from connection 2. This should fail as 2
  // should not be able to access 1.
  ASSERT_FALSE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(1, 2)));

  // Try to reparent 1 to the root. A connection is not allowed to reparent its
  // roots.
  ASSERT_FALSE(AddView(vm2(), BuildViewId(0, 1), BuildViewId(1, 1)));
}

// Verify RemoveViewFromParent fails for views that are descendants of the
// roots.
TEST_F(ViewManagerServiceAppTest, CantRemoveViewsInOtherRoots) {
  // Create 1 and 2 in the first connection and parent both to the root.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));

  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 2)));

  // Establish the second connection and give it the root 1.
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(false));

  // Connection 2 should not be able to remove view 2 or 1 from its parent.
  ASSERT_FALSE(RemoveViewFromParent(vm2(), BuildViewId(1, 2)));
  ASSERT_FALSE(RemoveViewFromParent(vm2(), BuildViewId(1, 1)));

  // Create views 10 and 11 in 2.
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 10)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 11)));

  // Parent 11 to 10.
  ASSERT_TRUE(AddView(vm2(), BuildViewId(2, 10), BuildViewId(2, 11)));
  // Remove 11 from 10.
  ASSERT_TRUE(RemoveViewFromParent(vm2(), BuildViewId(2, 11)));

  // Verify nothing was actually removed.
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(0, 1), &views);
    ASSERT_EQ(3u, views.size());
    EXPECT_EQ("view=0,1 parent=null", views[0].ToString());
    EXPECT_EQ("view=1,1 parent=0,1", views[1].ToString());
    EXPECT_EQ("view=1,2 parent=0,1", views[2].ToString());
  }
}

// Verify GetViewTree fails for views that are not descendants of the roots.
TEST_F(ViewManagerServiceAppTest, CantGetViewTreeOfOtherRoots) {
  // Create 1 and 2 in the first connection and parent both to the root.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));

  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 2)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(false));

  std::vector<TestView> views;

  // Should get nothing for the root.
  GetViewTree(vm2(), BuildViewId(0, 1), &views);
  ASSERT_TRUE(views.empty());

  // Should get nothing for view 2.
  GetViewTree(vm2(), BuildViewId(1, 2), &views);
  ASSERT_TRUE(views.empty());

  // Should get view 1 if asked for.
  GetViewTree(vm2(), BuildViewId(1, 1), &views);
  ASSERT_EQ(1u, views.size());
  EXPECT_EQ("view=1,1 parent=null", views[0].ToString());
}

TEST_F(ViewManagerServiceAppTest, OnViewInputEvent) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  changes2()->clear();

  // Dispatch an event to the view and verify it's received.
  {
    EventPtr event(Event::New());
    event->action = static_cast<EventType>(1);
    wm_internal_->DispatchInputEventToView(BuildViewId(1, 1), event.Pass());
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("InputEvent view=1,1 event_action=1",
              SingleChangeToDescription(*changes2()));
  }
}

TEST_F(ViewManagerServiceAppTest, EmbedWithSameViewId) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  changes2()->clear();

  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm1(), BuildViewId(1, 1)));

  // Connection2 should have been told the view was deleted.
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("ViewDeleted view=1,1", SingleChangeToDescription(*changes2()));
  }

  // Connection2 has no root. Verify it can't see view 1,1 anymore.
  {
    std::vector<TestView> views;
    GetViewTree(vm2(), BuildViewId(1, 1), &views);
    EXPECT_TRUE(views.empty());
  }
}

TEST_F(ViewManagerServiceAppTest, EmbedWithSameViewId2) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  changes2()->clear();

  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm1(), BuildViewId(1, 1)));

  // Connection2 should have been told the view was deleted.
  vm_client2_->WaitForChangeCount(1);
  changes2()->clear();

  // Create a view in the third connection and parent it to the root.
  ASSERT_TRUE(CreateView(vm3(), BuildViewId(3, 1)));
  ASSERT_TRUE(AddView(vm3(), BuildViewId(1, 1), BuildViewId(3, 1)));

  // Connection 1 should have been told about the add (it owns the view).
  {
    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("HierarchyChanged view=3,1 new_parent=1,1 old_parent=null",
              SingleChangeToDescription(*changes1()));
  }

  // Embed 1,1 again.
  {
    changes3()->clear();

    // We should get a new connection for the new embedding.
    scoped_ptr<ViewManagerClientImpl> connection4(
        EstablishConnectionViaEmbed(vm1(), BuildViewId(1, 1)));
    ASSERT_TRUE(connection4.get());
    EXPECT_EQ("[view=1,1 parent=null]",
              ChangeViewDescription(*connection4->tracker()->changes()));

    // And 3 should get a delete.
    vm_client3_->WaitForChangeCount(1);
    EXPECT_EQ("ViewDeleted view=1,1", SingleChangeToDescription(*changes3()));
  }

  // vm3() has no root. Verify it can't see view 1,1 anymore.
  {
    std::vector<TestView> views;
    GetViewTree(vm3(), BuildViewId(1, 1), &views);
    EXPECT_TRUE(views.empty());
  }

  // Verify 3,1 is no longer parented to 1,1. We have to do this from 1,1 as
  // vm3() can no longer see 1,1.
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 1), &views);
    ASSERT_EQ(1u, views.size());
    EXPECT_EQ("view=1,1 parent=null", views[0].ToString());
  }

  // Verify vm3() can still see the view it created 3,1.
  {
    std::vector<TestView> views;
    GetViewTree(vm3(), BuildViewId(3, 1), &views);
    ASSERT_EQ(1u, views.size());
    EXPECT_EQ("view=3,1 parent=null", views[0].ToString());
  }
}

// Assertions for SetViewVisibility.
TEST_F(ViewManagerServiceAppTest, SetViewVisibility) {
  // Create 1 and 2 in the first connection and parent both to the root.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));

  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(0, 1), &views);
    ASSERT_EQ(2u, views.size());
    EXPECT_EQ("view=0,1 parent=null visible=true drawn=true",
              views[0].ToString2());
    EXPECT_EQ("view=1,1 parent=0,1 visible=true drawn=true",
              views[1].ToString2());
  }

  // Hide 1.
  ASSERT_TRUE(SetViewVisibility(vm1(), BuildViewId(1, 1), false));
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 1), &views);
    ASSERT_EQ(1u, views.size());
    EXPECT_EQ("view=1,1 parent=0,1 visible=false drawn=false",
              views[0].ToString2());
  }

  // Attach 2 to 1.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 1), BuildViewId(1, 2)));
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 1), &views);
    ASSERT_EQ(2u, views.size());
    EXPECT_EQ("view=1,1 parent=0,1 visible=false drawn=false",
              views[0].ToString2());
    EXPECT_EQ("view=1,2 parent=1,1 visible=true drawn=false",
              views[1].ToString2());
  }

  // Show 1.
  ASSERT_TRUE(SetViewVisibility(vm1(), BuildViewId(1, 1), true));
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 1), &views);
    ASSERT_EQ(2u, views.size());
    EXPECT_EQ("view=1,1 parent=0,1 visible=true drawn=true",
              views[0].ToString2());
    EXPECT_EQ("view=1,2 parent=1,1 visible=true drawn=true",
              views[1].ToString2());
  }
}

// Assertions for SetViewVisibility sending notifications.
TEST_F(ViewManagerServiceAppTest, SetViewVisibilityNotifications) {
  // Create 1,1 and 1,2. 1,2 is made a child of 1,1 and 1,1 a child of the root.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 2)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(1, 1), BuildViewId(1, 2)));

  // Establish the second connection at 1,2.
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnectionWithRoot(BuildViewId(1, 2)));

  // Add 2,3 as a child of 1,2.
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 3)));
  changes1()->clear();
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 2), BuildViewId(2, 3)));
  vm_client1_.WaitForChangeCount(1);

  changes2()->clear();
  // Hide 1,2 from connection 1. Connection 2 should see this.
  ASSERT_TRUE(SetViewVisibility(vm1(), BuildViewId(1, 2), false));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("VisibilityChanged view=1,2 visible=false",
              SingleChangeToDescription(*changes2()));
  }

  changes1()->clear();
  // Show 1,2 from connection 2, connection 1 should be notified.
  ASSERT_TRUE(SetViewVisibility(vm2(), BuildViewId(1, 2), true));
  {
    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("VisibilityChanged view=1,2 visible=true",
              SingleChangeToDescription(*changes1()));
  }

  changes2()->clear();
  // Hide 1,1, connection 2 should be told the draw state changed.
  ASSERT_TRUE(SetViewVisibility(vm1(), BuildViewId(1, 1), false));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,2 drawn=false",
              SingleChangeToDescription(*changes2()));
  }

  changes2()->clear();
  // Show 1,1 from connection 1. Connection 2 should see this.
  ASSERT_TRUE(SetViewVisibility(vm1(), BuildViewId(1, 1), true));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,2 drawn=true",
              SingleChangeToDescription(*changes2()));
  }

  // Change visibility of 2,3, connection 1 should see this.
  changes1()->clear();
  ASSERT_TRUE(SetViewVisibility(vm2(), BuildViewId(2, 3), false));
  {
    vm_client1_.WaitForChangeCount(1);
    EXPECT_EQ("VisibilityChanged view=2,3 visible=false",
              SingleChangeToDescription(*changes1()));
  }

  changes2()->clear();
  // Remove 1,1 from the root, connection 2 should see drawn state changed.
  ASSERT_TRUE(RemoveViewFromParent(vm1(), BuildViewId(1, 1)));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,2 drawn=false",
              SingleChangeToDescription(*changes2()));
  }

  changes2()->clear();
  // Add 1,1 back to the root, connection 2 should see drawn state changed.
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,2 drawn=true",
              SingleChangeToDescription(*changes2()));
  }
}

TEST_F(ViewManagerServiceAppTest, SetViewProperty) {
  // Create 1 and 2 in the first connection and parent both to the root.
  ASSERT_TRUE(CreateView(vm1(), BuildViewId(1, 1)));

  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(false));
  changes2()->clear();

  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(0, 1), &views);
    ASSERT_EQ(2u, views.size());
    EXPECT_EQ("view=0,1 parent=null visible=true drawn=true",
              views[0].ToString2());
    EXPECT_EQ("view=1,1 parent=0,1 visible=true drawn=true",
              views[1].ToString2());

    ASSERT_EQ(0u, views[1].properties.size());

    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("DrawnStateChanged view=1,1 drawn=true",
              SingleChangeToDescription(*changes2()));
  }

  // Set properties on 1.
  changes2()->clear();
  std::vector<uint8_t> one(1, '1');
  ASSERT_TRUE(SetViewProperty(vm1(), BuildViewId(1, 1), "one", &one));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("PropertyChanged view=1,1 key=one value=1",
              SingleChangeToDescription(*changes2()));
  }

  // Test that our properties exist in the view tree
  {
    std::vector<TestView> views;
    GetViewTree(vm1(), BuildViewId(1, 1), &views);
    ASSERT_EQ(1u, views.size());
    ASSERT_EQ(1u, views[0].properties.size());
    EXPECT_EQ(one, views[0].properties["one"]);
  }

  changes2()->clear();
  // Set back to null.
  ASSERT_TRUE(SetViewProperty(vm1(), BuildViewId(1, 1), "one", NULL));
  {
    vm_client2_->WaitForChangeCount(1);
    EXPECT_EQ("PropertyChanged view=1,1 key=one value=NULL",
              SingleChangeToDescription(*changes2()));
  }
}

TEST_F(ViewManagerServiceAppTest, OnEmbeddedAppDisconnected) {
  // Create connection 2 and 3.
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));
  changes2()->clear();
  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm2(), BuildViewId(2, 2)));

  // Close connection 3. Connection 2 (which had previously embedded 3) should
  // be notified of this.
  vm_client3_.reset();
  vm_client2_->WaitForChangeCount(1);
  EXPECT_EQ("OnEmbeddedAppDisconnected view=2,2",
            SingleChangeToDescription(*changes2()));
}

// Verifies when the parent of an Embed() is destroyed the embedded app gets
// a ViewDeleted (and doesn't trigger a DCHECK).
TEST_F(ViewManagerServiceAppTest, OnParentOfEmbedDisconnects) {
  // Create connection 2 and 3.
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  ASSERT_TRUE(AddView(vm1(), BuildViewId(0, 1), BuildViewId(1, 1)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 2)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(1, 1), BuildViewId(2, 2)));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 3)));
  ASSERT_TRUE(AddView(vm2(), BuildViewId(2, 2), BuildViewId(2, 3)));
  changes2()->clear();
  ASSERT_NO_FATAL_FAILURE(EstablishThirdConnection(vm2(), BuildViewId(2, 3)));
  changes3()->clear();

  // Close connection 2. Connection 3 should get a delete (for its root).
  vm_client2_.reset();
  vm_client3_->WaitForChangeCount(1);
  EXPECT_EQ("ViewDeleted view=2,3", SingleChangeToDescription(*changes3()));
}

// Verifies ViewManagerServiceImpl doesn't incorrectly erase from its internal
// map when a view from another connection with the same view_id is removed.
TEST_F(ViewManagerServiceAppTest, DontCleanMapOnDestroy) {
  ASSERT_NO_FATAL_FAILURE(EstablishSecondConnection(true));
  ASSERT_TRUE(CreateView(vm2(), BuildViewId(2, 1)));
  changes1()->clear();
  vm_client2_.reset();
  vm_client1_.WaitForChangeCount(1);
  EXPECT_EQ("OnEmbeddedAppDisconnected view=1,1",
            SingleChangeToDescription(*changes1()));
  std::vector<TestView> views;
  GetViewTree(vm1(), BuildViewId(1, 1), &views);
  EXPECT_FALSE(views.empty());
}

// TODO(sky): need to better track changes to initial connection. For example,
// that SetBounsdViews/AddView and the like don't result in messages to the
// originating connection.

// TODO(sky): make sure coverage of what was
// ViewManagerTest.SecondEmbedRoot_InitService and
// ViewManagerTest.MultipleEmbedRootsBeforeWTHReady gets added to window manager
// tests.

}  // namespace service
}  // namespace mojo
