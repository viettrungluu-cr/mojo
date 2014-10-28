// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/view_manager/connection_manager.h"

#include "base/logging.h"
#include "base/stl_util.h"
#include "mojo/converters/input_events/input_events_type_converters.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "mojo/services/view_manager/connection_manager_delegate.h"
#include "mojo/services/view_manager/view_manager_service_impl.h"

namespace mojo {
namespace service {

class WindowManagerInternalClientImpl
    : public InterfaceImpl<WindowManagerInternalClient> {
 public:
  WindowManagerInternalClientImpl(WindowManagerInternalClient* real_client,
                                  ErrorHandler* error_handler)
      : real_client_(real_client), error_handler_(error_handler) {}
  ~WindowManagerInternalClientImpl() override {}

  // WindowManagerInternalClient:
  void DispatchInputEventToView(Id transport_view_id, EventPtr event) override {
    real_client_->DispatchInputEventToView(transport_view_id, event.Pass());
  }

  // InterfaceImpl:
  void OnConnectionError() override { error_handler_->OnConnectionError(); }

 private:
  WindowManagerInternalClient* real_client_;
  ErrorHandler* error_handler_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerInternalClientImpl);
};

ConnectionManager::ScopedChange::ScopedChange(
    ViewManagerServiceImpl* connection,
    ConnectionManager* connection_manager,
    bool is_delete_view)
    : connection_manager_(connection_manager),
      connection_id_(connection->id()),
      is_delete_view_(is_delete_view) {
  connection_manager_->PrepareForChange(this);
}

ConnectionManager::ScopedChange::~ScopedChange() {
  connection_manager_->FinishChange();
}

ConnectionManager::ConnectionManager(ApplicationConnection* app_connection,
                                     ConnectionManagerDelegate* delegate)
    : app_connection_(app_connection),
      delegate_(delegate),
      window_manager_vm_service_(nullptr),
      next_connection_id_(1),
      display_manager_(
          app_connection,
          this,
          base::Bind(&ConnectionManagerDelegate::OnNativeViewportDestroyed,
                     base::Unretained(delegate))),
      root_(new ServerView(this, RootViewId())),
      current_change_(NULL),
      in_destructor_(false) {
  // |app_connection| originates from the WindowManager. Let it connect
  // directly to the ViewManager and WindowManagerInternalClient.
  app_connection->AddService(
      static_cast<InterfaceFactory<ViewManagerService>*>(this));
  app_connection->AddService(
      static_cast<InterfaceFactory<WindowManagerInternalClient>*>(this));
  root_->SetBounds(gfx::Rect(800, 600));
}

ConnectionManager::~ConnectionManager() {
  in_destructor_ = true;

  STLDeleteValues(&connection_map_);
  // All the connections should have been destroyed.
  DCHECK(connection_map_.empty());
  root_.reset();
}

ConnectionSpecificId ConnectionManager::GetAndAdvanceNextConnectionId() {
  const ConnectionSpecificId id = next_connection_id_++;
  DCHECK_LT(id, next_connection_id_);
  return id;
}

void ConnectionManager::OnConnectionError(ViewManagerServiceImpl* connection) {
  scoped_ptr<ViewManagerServiceImpl> connection_owner(connection);

  connection_map_.erase(connection->id());

  // Notify remaining connections so that they can cleanup.
  for (ConnectionMap::const_iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->OnViewManagerServiceImplDestroyed(connection->id());
  }

  if (connection == window_manager_vm_service_) {
    window_manager_vm_service_ = nullptr;
    delegate_->OnLostConnectionToWindowManager();
  }
}

void ConnectionManager::EmbedAtView(
    ConnectionSpecificId creator_id,
    const String& url,
    Id transport_view_id,
    InterfaceRequest<ServiceProvider> service_provider) {
  MessagePipe pipe;

  ServiceProvider* view_manager_service_provider =
      app_connection_->ConnectToApplication(url)->GetServiceProvider();

  view_manager_service_provider->ConnectToService(
      ViewManagerServiceImpl::Client::Name_, pipe.handle1.Pass());

  std::string creator_url;
  ConnectionMap::const_iterator it = connection_map_.find(creator_id);
  if (it != connection_map_.end())
    creator_url = it->second->url();

  ViewManagerServiceImpl* connection =
      new ViewManagerServiceImpl(this,
                                 creator_id,
                                 creator_url,
                                 url.To<std::string>(),
                                 ViewIdFromTransportId(transport_view_id),
                                 service_provider.Pass());
  AddConnection(connection);
  WeakBindToPipe(connection, pipe.handle0.Pass());
  OnConnectionMessagedClient(connection->id());
}

ViewManagerServiceImpl* ConnectionManager::GetConnection(
    ConnectionSpecificId connection_id) {
  ConnectionMap::iterator i = connection_map_.find(connection_id);
  return i == connection_map_.end() ? NULL : i->second;
}

ServerView* ConnectionManager::GetView(const ViewId& id) {
  if (id == root_->id())
    return root_.get();
  ConnectionMap::iterator i = connection_map_.find(id.connection_id);
  return i == connection_map_.end() ? NULL : i->second->GetView(id);
}

void ConnectionManager::OnConnectionMessagedClient(ConnectionSpecificId id) {
  if (current_change_)
    current_change_->MarkConnectionAsMessaged(id);
}

bool ConnectionManager::DidConnectionMessageClient(
    ConnectionSpecificId id) const {
  return current_change_ && current_change_->DidMessageConnection(id);
}

const ViewManagerServiceImpl* ConnectionManager::GetConnectionWithRoot(
    const ViewId& id) const {
  for (ConnectionMap::const_iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    if (i->second->HasRoot(id))
      return i->second;
  }
  return NULL;
}

void ConnectionManager::ProcessViewBoundsChanged(const ServerView* view,
                                                 const gfx::Rect& old_bounds,
                                                 const gfx::Rect& new_bounds) {
  for (ConnectionMap::iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->ProcessViewBoundsChanged(
        view, old_bounds, new_bounds, IsChangeSource(i->first));
  }
}

void ConnectionManager::ProcessWillChangeViewHierarchy(
    const ServerView* view,
    const ServerView* new_parent,
    const ServerView* old_parent) {
  for (ConnectionMap::iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->ProcessWillChangeViewHierarchy(
        view, new_parent, old_parent, IsChangeSource(i->first));
  }
}

void ConnectionManager::ProcessViewHierarchyChanged(
    const ServerView* view,
    const ServerView* new_parent,
    const ServerView* old_parent) {
  for (ConnectionMap::iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->ProcessViewHierarchyChanged(
        view, new_parent, old_parent, IsChangeSource(i->first));
  }
}

void ConnectionManager::ProcessViewReorder(const ServerView* view,
                                           const ServerView* relative_view,
                                           const OrderDirection direction) {
  for (ConnectionMap::iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->ProcessViewReorder(
        view, relative_view, direction, IsChangeSource(i->first));
  }
}

void ConnectionManager::ProcessViewDeleted(const ViewId& view) {
  for (ConnectionMap::iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->ProcessViewDeleted(view, IsChangeSource(i->first));
  }
}

void ConnectionManager::PrepareForChange(ScopedChange* change) {
  // Should only ever have one change in flight.
  CHECK(!current_change_);
  current_change_ = change;
}

void ConnectionManager::FinishChange() {
  // PrepareForChange/FinishChange should be balanced.
  CHECK(current_change_);
  current_change_ = NULL;
}

void ConnectionManager::AddConnection(ViewManagerServiceImpl* connection) {
  DCHECK_EQ(0u, connection_map_.count(connection->id()));
  connection_map_[connection->id()] = connection;
}

void ConnectionManager::OnViewDestroyed(const ServerView* view) {
  if (!in_destructor_)
    ProcessViewDeleted(view->id());
}

void ConnectionManager::OnWillChangeViewHierarchy(
    const ServerView* view,
    const ServerView* new_parent,
    const ServerView* old_parent) {
  if (!in_destructor_ && !display_manager_.in_setup())
    ProcessWillChangeViewHierarchy(view, new_parent, old_parent);
}

void ConnectionManager::OnViewHierarchyChanged(const ServerView* view,
                                               const ServerView* new_parent,
                                               const ServerView* old_parent) {
  if (in_destructor_)
    return;

  if (!display_manager_.in_setup())
    ProcessViewHierarchyChanged(view, new_parent, old_parent);

  // TODO(beng): optimize.
  if (old_parent) {
    display_manager_.SchedulePaint(old_parent,
                                   gfx::Rect(old_parent->bounds().size()));
  }
  if (new_parent) {
    display_manager_.SchedulePaint(new_parent,
                                   gfx::Rect(new_parent->bounds().size()));
  }
}

void ConnectionManager::OnViewBoundsChanged(const ServerView* view,
                                            const gfx::Rect& old_bounds,
                                            const gfx::Rect& new_bounds) {
  if (in_destructor_)
    return;

  ProcessViewBoundsChanged(view, old_bounds, new_bounds);
  if (!view->parent())
    return;

  // TODO(sky): optimize this.
  display_manager_.SchedulePaint(view->parent(), old_bounds);
  display_manager_.SchedulePaint(view->parent(), new_bounds);
}

void ConnectionManager::OnViewSurfaceIdChanged(const ServerView* view) {
  if (!in_destructor_)
    display_manager_.SchedulePaint(view, gfx::Rect(view->bounds().size()));
}

void ConnectionManager::OnViewReordered(const ServerView* view,
                                        const ServerView* relative,
                                        OrderDirection direction) {
  if (!in_destructor_)
    display_manager_.SchedulePaint(view, gfx::Rect(view->bounds().size()));
}

void ConnectionManager::OnWillChangeViewVisibility(const ServerView* view) {
  if (in_destructor_)
    return;

  for (ConnectionMap::iterator i = connection_map_.begin();
       i != connection_map_.end();
       ++i) {
    i->second->ProcessWillChangeViewVisibility(view, IsChangeSource(i->first));
  }
}

void ConnectionManager::OnViewPropertyChanged(
    const ServerView* view,
    const std::string& name,
    const std::vector<uint8_t>* new_data) {
  for (auto& pair : connection_map_) {
    pair.second->ProcessViewPropertyChanged(
        view, name, new_data, IsChangeSource(pair.first));
  }
}

void ConnectionManager::DispatchInputEventToView(Id transport_view_id,
                                                 EventPtr event) {
  const ViewId view_id(ViewIdFromTransportId(transport_view_id));

  ViewManagerServiceImpl* connection = GetConnectionWithRoot(view_id);
  if (!connection)
    connection = GetConnection(view_id.connection_id);
  if (connection) {
    connection->client()->OnViewInputEvent(
        transport_view_id, event.Pass(), base::Bind(&base::DoNothing));
  }
}

void ConnectionManager::Create(ApplicationConnection* connection,
                               InterfaceRequest<ViewManagerService> request) {
  if (window_manager_vm_service_) {
    VLOG(1) << "ViewManager interface requested more than once.";
    return;
  }

  window_manager_vm_service_ =
      new ViewManagerServiceImpl(this,
                                 kInvalidConnectionId,
                                 std::string(),
                                 std::string("mojo:window_manager"),
                                 RootViewId(),
                                 InterfaceRequest<ServiceProvider>());
  AddConnection(window_manager_vm_service_);
  WeakBindToRequest(window_manager_vm_service_, &request);
}

void ConnectionManager::Create(
    ApplicationConnection* connection,
    InterfaceRequest<WindowManagerInternalClient> request) {
  if (wm_internal_client_impl_.get()) {
    VLOG(1) << "WindowManagerInternalClient requested more than once.";
    return;
  }

  wm_internal_client_impl_.reset(
      new WindowManagerInternalClientImpl(this, this));
  WeakBindToRequest(wm_internal_client_impl_.get(), &request);
}

void ConnectionManager::OnConnectionError() {
  delegate_->OnLostConnectionToWindowManager();
}

}  // namespace service
}  // namespace mojo
