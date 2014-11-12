// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_SERVICE_IMPL_H_
#define MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_SERVICE_IMPL_H_

#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/containers/hash_tables.h"
#include "base/memory/scoped_ptr.h"
#include "mojo/services/public/interfaces/surfaces/surface_id.mojom.h"
#include "mojo/services/public/interfaces/view_manager/view_manager.mojom.h"
#include "mojo/services/view_manager/access_policy_delegate.h"
#include "mojo/services/view_manager/ids.h"

namespace gfx {
class Rect;
}

namespace mojo {
namespace service {

class AccessPolicy;
class ConnectionManager;
class ServerView;

// Manages a connection from the client.
class ViewManagerServiceImpl
    : public InterfaceImpl<ViewManagerService>,
      public AccessPolicyDelegate {
 public:
  using ViewIdSet = base::hash_set<Id>;

  ViewManagerServiceImpl(ConnectionManager* connection_manager,
                         ConnectionSpecificId creator_id,
                         const std::string& creator_url,
                         const std::string& url,
                         const ViewId& root_id);
  ~ViewManagerServiceImpl() override;

  // Called after bound. |service_provider| is the ServiceProvider to pass to
  // the client via OnEmbed().
  void Init(InterfaceRequest<ServiceProvider> service_provider);

  ConnectionSpecificId id() const { return id_; }
  ConnectionSpecificId creator_id() const { return creator_id_; }
  const std::string& url() const { return url_; }

  // Returns the View with the specified id.
  ServerView* GetView(const ViewId& id) {
    return const_cast<ServerView*>(
        const_cast<const ViewManagerServiceImpl*>(this)->GetView(id));
  }
  const ServerView* GetView(const ViewId& id) const;

  // Returns true if this connection's root is |id|.
  bool IsRoot(const ViewId& id) const;

  // Invoked when a connection is about to be destroyed.
  void OnWillDestroyViewManagerServiceImpl(ViewManagerServiceImpl* connection);

  // The following methods are invoked after the corresponding change has been
  // processed. They do the appropriate bookkeeping and update the client as
  // necessary.
  void ProcessViewBoundsChanged(const ServerView* view,
                                const gfx::Rect& old_bounds,
                                const gfx::Rect& new_bounds,
                                bool originated_change);
  void ProcessWillChangeViewHierarchy(const ServerView* view,
                                      const ServerView* new_parent,
                                      const ServerView* old_parent,
                                      bool originated_change);
  void ProcessViewPropertyChanged(const ServerView* view,
                                  const std::string& name,
                                  const std::vector<uint8_t>* new_data,
                                  bool originated_change);
  void ProcessViewHierarchyChanged(const ServerView* view,
                                   const ServerView* new_parent,
                                   const ServerView* old_parent,
                                   bool originated_change);
  void ProcessViewReorder(const ServerView* view,
                          const ServerView* relative_view,
                          OrderDirection direction,
                          bool originated_change);
  void ProcessViewDeleted(const ViewId& view, bool originated_change);
  void ProcessWillChangeViewVisibility(const ServerView* view,
                                       bool originated_change);
  void ProcessViewPropertiesChanged(const ServerView* view,
                                    bool originated_change);

  // TODO(sky): move this to private section (currently can't because of
  // bindings).
  // InterfaceImp overrides:
  void OnConnectionError() override;

 private:
  typedef std::map<ConnectionSpecificId, ServerView*> ViewMap;

  bool IsViewKnown(const ServerView* view) const;

  // These functions return true if the corresponding mojom function is allowed
  // for this connection.
  bool CanReorderView(const ServerView* view,
                      const ServerView* relative_view,
                      OrderDirection direction) const;

  // Deletes a view owned by this connection. Returns true on success. |source|
  // is the connection that originated the change.
  bool DeleteViewImpl(ViewManagerServiceImpl* source, ServerView* view);

  // If |view| is known (in |known_views_|) does nothing. Otherwise adds |view|
  // to |views|, marks |view| as known and recurses.
  void GetUnknownViewsFrom(const ServerView* view,
                           std::vector<const ServerView*>* views);

  // Removes |view| and all its descendants from |known_views_|. This does not
  // recurse through views that were created by this connection. All views owned
  // by this connection are added to |local_views|.
  void RemoveFromKnown(const ServerView* view,
                       std::vector<ServerView*>* local_views);

  // Resets the root of this connection.
  void RemoveRoot();

  void RemoveChildrenAsPartOfEmbed(const ViewId& view_id);

  // Converts View(s) to ViewData(s) for transport. This assumes all the views
  // are valid for the client. The parent of views the client is not allowed to
  // see are set to NULL (in the returned ViewData(s)).
  Array<ViewDataPtr> ViewsToViewDatas(
      const std::vector<const ServerView*>& views);
  ViewDataPtr ViewToViewData(const ServerView* view);

  // Implementation of GetViewTree(). Adds |view| to |views| and recurses if
  // CanDescendIntoViewForViewTree() returns true.
  void GetViewTreeImpl(const ServerView* view,
                       std::vector<const ServerView*>* views) const;

  // Notify the client if the drawn state of any of the roots changes.
  // |view| is the view that is changing to the drawn state |new_drawn_value|.
  void NotifyDrawnStateChanged(const ServerView* view, bool new_drawn_value);

  // Deletes all Views we own.
  void DestroyViews();

  // ViewManagerService:
  void CreateView(Id transport_view_id,
                  const Callback<void(ErrorCode)>& callback) override;
  void DeleteView(Id transport_view_id,
                  const Callback<void(bool)>& callback) override;
  void AddView(Id parent_id,
               Id child_id,
               const Callback<void(bool)>& callback) override;
  void RemoveViewFromParent(Id view_id,
                            const Callback<void(bool)>& callback) override;
  void ReorderView(Id view_id,
                   Id relative_view_id,
                   OrderDirection direction,
                   const Callback<void(bool)>& callback) override;
  void GetViewTree(Id view_id,
                   const Callback<void(Array<ViewDataPtr>)>& callback) override;
  void SetViewSurfaceId(Id view_id,
                        SurfaceIdPtr surface_id,
                        const Callback<void(bool)>& callback) override;
  void SetViewBounds(Id view_id,
                     RectPtr bounds,
                     const Callback<void(bool)>& callback) override;
  void SetViewVisibility(Id view_id,
                         bool visible,
                         const Callback<void(bool)>& callback) override;
  void SetViewProperty(Id view_id,
                       const String& name,
                       Array<uint8_t> value,
                       const Callback<void(bool)>& callback) override;
  void Embed(const String& url,
             Id view_id,
             InterfaceRequest<ServiceProvider> service_provider,
             const Callback<void(bool)>& callback) override;

  // AccessPolicyDelegate:
  bool IsRootForAccessPolicy(const ViewId& id) const override;
  bool IsViewKnownForAccessPolicy(const ServerView* view) const override;
  bool IsViewRootOfAnotherConnectionForAccessPolicy(
      const ServerView* view) const override;

  ConnectionManager* connection_manager_;

  // Id of this connection as assigned by ConnectionManager.
  const ConnectionSpecificId id_;

  // URL this connection was created for.
  const std::string url_;

  // ID of the connection that created us. If 0 it indicates either we were
  // created by the root, or the connection that created us has been destroyed.
  ConnectionSpecificId creator_id_;

  // The URL of the app that embedded the app this connection was created for.
  const std::string creator_url_;

  scoped_ptr<AccessPolicy> access_policy_;

  // The views and views created by this connection. This connection owns these
  // objects.
  ViewMap view_map_;

  // The set of views that has been communicated to the client.
  ViewIdSet known_views_;

  // The root of this connection. This is a scoped_ptr to reinforce the
  // connection may have no root. A connection has no root if either the root
  // is destroyed or Embed() is invoked on the root.
  scoped_ptr<ViewId> root_;

  DISALLOW_COPY_AND_ASSIGN(ViewManagerServiceImpl);
};

}  // namespace service
}  // namespace mojo

#endif  // MOJO_SERVICES_VIEW_MANAGER_VIEW_MANAGER_SERVICE_IMPL_H_
