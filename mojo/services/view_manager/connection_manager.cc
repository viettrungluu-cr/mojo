// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/view_manager/connection_manager.h"

#include "base/logging.h"
#include "base/stl_util.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/converters/input_events/input_events_type_converters.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "mojo/services/view_manager/client_connection.h"
#include "mojo/services/view_manager/connection_manager_delegate.h"
#include "mojo/services/view_manager/display_manager.h"
#include "mojo/services/view_manager/view_manager_service_impl.h"

namespace mojo {
namespace service {

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

ConnectionManager::ConnectionManager(ConnectionManagerDelegate* delegate,
                                     scoped_ptr<DisplayManager> display_manager,
                                     WindowManagerInternal* wm_internal)
    : delegate_(delegate),
      window_manager_client_connection_(nullptr),
      next_connection_id_(1),
      display_manager_(display_manager.Pass()),
      root_(new ServerView(this, RootViewId())),
      wm_internal_(wm_internal),
      current_change_(nullptr),
      in_destructor_(false) {
  root_->SetBounds(gfx::Rect(800, 600));
  display_manager_->Init(this);
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

void ConnectionManager::OnConnectionError(ClientConnection* connection) {
  if (connection == window_manager_client_connection_) {
    window_manager_client_connection_ = nullptr;
    delegate_->OnLostConnectionToWindowManager();
    // Assume we've been destroyed.
    return;
  }

  scoped_ptr<ClientConnection> connection_owner(connection);

  connection_map_.erase(connection->service()->id());

  // Notify remaining connections so that they can cleanup.
  for (auto& pair : connection_map_) {
    pair.second->service()->OnWillDestroyViewManagerServiceImpl(
        connection->service());
  }
}

void ConnectionManager::EmbedAtView(
    ConnectionSpecificId creator_id,
    const String& url,
    Id transport_view_id,
    InterfaceRequest<ServiceProvider> service_provider) {
  std::string creator_url;
  ConnectionMap::const_iterator it = connection_map_.find(creator_id);
  if (it != connection_map_.end())
    creator_url = it->second->service()->url();

  ClientConnection* client_connection =
      delegate_->CreateClientConnectionForEmbedAtView(
          this, creator_id, creator_url, url.To<std::string>(),
          ViewIdFromTransportId(transport_view_id));
  AddConnection(client_connection);
  client_connection->service()->Init(client_connection->client(),
                                     service_provider.Pass());
  OnConnectionMessagedClient(client_connection->service()->id());
}

ViewManagerServiceImpl* ConnectionManager::GetConnection(
    ConnectionSpecificId connection_id) {
  ConnectionMap::iterator i = connection_map_.find(connection_id);
  return i == connection_map_.end() ? nullptr : i->second->service();
}

ServerView* ConnectionManager::GetView(const ViewId& id) {
  if (id == root_->id())
    return root_.get();
  ViewManagerServiceImpl* service = GetConnection(id.connection_id);
  return service ? service->GetView(id) : nullptr;
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
  for (auto& pair : connection_map_) {
    if (pair.second->service()->IsRoot(id))
      return pair.second->service();
  }
  return nullptr;
}

void ConnectionManager::SetWindowManagerClientConnection(
    scoped_ptr<ClientConnection> connection) {
  CHECK(!window_manager_client_connection_);
  window_manager_client_connection_ = connection.release();
  AddConnection(window_manager_client_connection_);
  window_manager_client_connection_->service()->Init(
      window_manager_client_connection_->client(),
      InterfaceRequest<ServiceProvider>());
}

void ConnectionManager::ProcessViewBoundsChanged(const ServerView* view,
                                                 const gfx::Rect& old_bounds,
                                                 const gfx::Rect& new_bounds) {
  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessViewBoundsChanged(
        view, old_bounds, new_bounds, IsChangeSource(pair.first));
  }
}

void ConnectionManager::ProcessWillChangeViewHierarchy(
    const ServerView* view,
    const ServerView* new_parent,
    const ServerView* old_parent) {
  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessWillChangeViewHierarchy(
        view, new_parent, old_parent, IsChangeSource(pair.first));
  }
}

void ConnectionManager::ProcessViewHierarchyChanged(
    const ServerView* view,
    const ServerView* new_parent,
    const ServerView* old_parent) {
  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessViewHierarchyChanged(
        view, new_parent, old_parent, IsChangeSource(pair.first));
  }
}

void ConnectionManager::ProcessViewReorder(const ServerView* view,
                                           const ServerView* relative_view,
                                           const OrderDirection direction) {
  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessViewReorder(view, relative_view, direction,
                                               IsChangeSource(pair.first));
  }
}

void ConnectionManager::ProcessViewDeleted(const ViewId& view) {
  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessViewDeleted(view,
                                               IsChangeSource(pair.first));
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

void ConnectionManager::AddConnection(ClientConnection* connection) {
  DCHECK_EQ(0u, connection_map_.count(connection->service()->id()));
  connection_map_[connection->service()->id()] = connection;
}

void ConnectionManager::OnViewDestroyed(const ServerView* view) {
  if (!in_destructor_)
    ProcessViewDeleted(view->id());
}

void ConnectionManager::OnWillChangeViewHierarchy(
    const ServerView* view,
    const ServerView* new_parent,
    const ServerView* old_parent) {
  if (!in_destructor_)
    ProcessWillChangeViewHierarchy(view, new_parent, old_parent);
}

void ConnectionManager::OnViewHierarchyChanged(const ServerView* view,
                                               const ServerView* new_parent,
                                               const ServerView* old_parent) {
  if (in_destructor_)
    return;

  ProcessViewHierarchyChanged(view, new_parent, old_parent);

  // TODO(beng): optimize.
  if (old_parent) {
    display_manager_->SchedulePaint(old_parent,
                                    gfx::Rect(old_parent->bounds().size()));
  }
  if (new_parent) {
    display_manager_->SchedulePaint(new_parent,
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
  display_manager_->SchedulePaint(view->parent(), old_bounds);
  display_manager_->SchedulePaint(view->parent(), new_bounds);
}

void ConnectionManager::OnViewSurfaceIdChanged(const ServerView* view) {
  if (!in_destructor_)
    display_manager_->SchedulePaint(view, gfx::Rect(view->bounds().size()));
}

void ConnectionManager::OnViewReordered(const ServerView* view,
                                        const ServerView* relative,
                                        OrderDirection direction) {
  if (!in_destructor_)
    display_manager_->SchedulePaint(view, gfx::Rect(view->bounds().size()));
}

void ConnectionManager::OnWillChangeViewVisibility(const ServerView* view) {
  if (in_destructor_)
    return;

  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessWillChangeViewVisibility(
        view, IsChangeSource(pair.first));
  }
}

void ConnectionManager::OnViewPropertyChanged(
    const ServerView* view,
    const std::string& name,
    const std::vector<uint8_t>* new_data) {
  for (auto& pair : connection_map_) {
    pair.second->service()->ProcessViewPropertyChanged(
        view, name, new_data, IsChangeSource(pair.first));
  }
}

void ConnectionManager::SetViewportSize(SizePtr size) {
  gfx::Size new_size = size.To<gfx::Size>();
  display_manager_->SetViewportSize(new_size);
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

}  // namespace service
}  // namespace mojo
