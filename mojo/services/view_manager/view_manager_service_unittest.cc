// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "mojo/common/common_type_converters.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "mojo/services/public/cpp/view_manager/types.h"
#include "mojo/services/public/cpp/view_manager/util.h"
#include "mojo/services/public/interfaces/view_manager/view_manager.mojom.h"
#include "mojo/services/public/interfaces/window_manager/window_manager.mojom.h"
#include "mojo/services/public/interfaces/window_manager/window_manager_internal.mojom.h"
#include "mojo/services/view_manager/client_connection.h"
#include "mojo/services/view_manager/connection_manager.h"
#include "mojo/services/view_manager/connection_manager_delegate.h"
#include "mojo/services/view_manager/display_manager.h"
#include "mojo/services/view_manager/ids.h"
#include "mojo/services/view_manager/test_change_tracker.h"
#include "mojo/services/view_manager/view_manager_service_impl.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/rect.h"

namespace mojo {
namespace service {
namespace {

// -----------------------------------------------------------------------------

// ViewManagerClient implementation that logs all calls to a TestChangeTracker.
// TODO(sky): refactor so both this and ViewManagerServiceAppTest share code.
class TestViewManagerClient : public ViewManagerClient {
 public:
  TestViewManagerClient() {}
  ~TestViewManagerClient() override {}

  TestChangeTracker* tracker() { return &tracker_; }

 private:
  // ViewManagerClient:
  void OnEmbed(uint16_t connection_id,
               const String& embedder_url,
               ViewDataPtr root,
               InterfaceRequest<ServiceProvider> parent_service_provider,
               ScopedMessagePipeHandle window_manager_pipe) override {
    tracker_.OnEmbed(connection_id, embedder_url, root.Pass());
  }
  void OnEmbeddedAppDisconnected(uint32_t view) override {
    tracker_.OnEmbeddedAppDisconnected(view);
  }
  void OnViewBoundsChanged(uint32_t view,
                           RectPtr old_bounds,
                           RectPtr new_bounds) override {
    tracker_.OnViewBoundsChanged(view, old_bounds.Pass(), new_bounds.Pass());
  }
  void OnViewHierarchyChanged(uint32_t view,
                              uint32_t new_parent,
                              uint32_t old_parent,
                              Array<ViewDataPtr> views) override {
    tracker_.OnViewHierarchyChanged(view, new_parent, old_parent, views.Pass());
  }
  void OnViewReordered(uint32_t view_id,
                       uint32_t relative_view_id,
                       OrderDirection direction) override {
    tracker_.OnViewReordered(view_id, relative_view_id, direction);
  }
  void OnViewDeleted(uint32_t view) override { tracker_.OnViewDeleted(view); }
  void OnViewVisibilityChanged(uint32_t view, bool visible) override {
    tracker_.OnViewVisibilityChanged(view, visible);
  }
  void OnViewDrawnStateChanged(uint32_t view, bool drawn) override {
    tracker_.OnViewDrawnStateChanged(view, drawn);
  }
  void OnViewSharedPropertyChanged(uint32_t view,
                                   const String& name,
                                   Array<uint8_t> new_data) override {
    tracker_.OnViewSharedPropertyChanged(view, name, new_data.Pass());
  }
  void OnViewInputEvent(uint32_t view,
                        EventPtr event,
                        const Callback<void()>& callback) override {
    tracker_.OnViewInputEvent(view, event.Pass());
  }

  TestChangeTracker tracker_;

  DISALLOW_COPY_AND_ASSIGN(TestViewManagerClient);
};

// -----------------------------------------------------------------------------

// ClientConnection implementation that vends TestViewManagerClient.
class TestClientConnection : public ClientConnection {
 public:
  explicit TestClientConnection(scoped_ptr<ViewManagerServiceImpl> service_impl)
      : ClientConnection(service_impl.Pass()) {
    set_client(&client_);
  }
  ~TestClientConnection() override {}

  TestViewManagerClient* client() { return &client_; }

 private:
  TestViewManagerClient client_;

  DISALLOW_COPY_AND_ASSIGN(TestClientConnection);
};

// -----------------------------------------------------------------------------

// Empty implementation of ConnectionManagerDelegate.
class TestConnectionManagerDelegate : public ConnectionManagerDelegate {
 public:
  TestConnectionManagerDelegate() : last_client_(nullptr) {}
  ~TestConnectionManagerDelegate() override {}

  // TestViewManagerClient that was created by the last
  // CreateClientConnectionForEmbedAtView() call.
  TestViewManagerClient* last_client() { return last_client_; }

 private:
  // ConnectionManagerDelegate:
  void OnLostConnectionToWindowManager() override {}

  ClientConnection* CreateClientConnectionForEmbedAtView(
      ConnectionManager* connection_manager,
      ConnectionSpecificId creator_id,
      const std::string& creator_url,
      const std::string& url,
      const ViewId& root_id) override {
    scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
        connection_manager, creator_id, creator_url, url, root_id));
    TestClientConnection* connection = new TestClientConnection(service.Pass());
    last_client_ = connection->client();
    return connection;
  }

  TestViewManagerClient* last_client_;

  DISALLOW_COPY_AND_ASSIGN(TestConnectionManagerDelegate);
};

// -----------------------------------------------------------------------------

// Empty implementation of DisplayManager.
class TestDisplayManager : public DisplayManager {
 public:
  TestDisplayManager() {}
  ~TestDisplayManager() override {}

  // DisplayManager:
  void Init(ConnectionManager* connection_manager) override {}
  void SchedulePaint(const ServerView* view, const gfx::Rect& bounds) override {
  }
  void SetViewportSize(const gfx::Size& size) override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(TestDisplayManager);
};

// -----------------------------------------------------------------------------

// Empty implementation of WindowManagerInternal.
class TestWindowManagerInternal : public WindowManagerInternal {
 public:
  TestWindowManagerInternal() {}
  ~TestWindowManagerInternal() override {}

  // WindowManagerInternal:
  void CreateWindowManagerForViewManagerClient(
      uint16_t connection_id,
      ScopedMessagePipeHandle window_manager_pipe) override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(TestWindowManagerInternal);
};

}  // namespace

// -----------------------------------------------------------------------------

class ViewManagerServiceTest : public testing::Test {
 public:
  ViewManagerServiceTest() : wm_client_(nullptr) {}
  ~ViewManagerServiceTest() override {}

 protected:
  // testing::Test:
  void SetUp() override {
    connection_manager_.reset(new ConnectionManager(
        &delegate_, scoped_ptr<DisplayManager>(new TestDisplayManager),
        &wm_internal_));

    scoped_ptr<ViewManagerServiceImpl> service(new ViewManagerServiceImpl(
        connection_manager_.get(), kInvalidConnectionId, std::string(),
        std::string("mojo:window_manager"), RootViewId()));
    scoped_ptr<TestClientConnection> client_connection(
        new TestClientConnection(service.Pass()));
    wm_client_ = client_connection->client();
    connection_manager_->SetWindowManagerClientConnection(
        client_connection.Pass());
  }

  // ViewManagerServiceImpl for the window manager.
  ViewManagerServiceImpl* wm_connection() {
    return connection_manager_->GetConnection(1);
  }

  // TestViewManagerClient that is used for the WM connection.
  TestViewManagerClient* wm_client_;

 private:
  TestWindowManagerInternal wm_internal_;
  TestConnectionManagerDelegate delegate_;
  scoped_ptr<ConnectionManager> connection_manager_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerServiceTest);
};

TEST_F(ViewManagerServiceTest, Basic) {
  wm_connection()->CreateView(ViewId(wm_connection()->id(), 1));
  ASSERT_EQ(1u, wm_client_->tracker()->changes()->size());
}

}  // namespace service
}  // namespace mojo
