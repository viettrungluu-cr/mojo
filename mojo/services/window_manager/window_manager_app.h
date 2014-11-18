// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_APP_H_
#define MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_APP_H_

#include <set>

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/public/cpp/bindings/string.h"
#include "mojo/services/public/cpp/view_manager/types.h"
#include "mojo/services/public/cpp/view_manager/view_manager_client_factory.h"
#include "mojo/services/public/cpp/view_manager/view_manager_delegate.h"
#include "mojo/services/public/cpp/view_manager/view_observer.h"
#include "mojo/services/public/interfaces/window_manager/window_manager_internal.mojom.h"
#include "mojo/services/window_manager/focus_controller_observer.h"
#include "mojo/services/window_manager/native_viewport_event_dispatcher_impl.h"
#include "mojo/services/window_manager/view_target.h"
#include "mojo/services/window_manager/window_manager_impl.h"
#include "ui/events/event_handler.h"

namespace gfx {
class Size;
}

namespace mojo {

class FocusController;
class FocusRules;
class WindowManagerClient;
class WindowManagerDelegate;
class WindowManagerImpl;
class ViewEventDispatcher;

// Implements core window manager functionality that could conceivably be shared
// across multiple window managers implementing superficially different user
// experiences. Establishes communication with the view manager.
// A window manager wishing to use this core should create and own an instance
// of this object. They may implement the associated ViewManager/WindowManager
// delegate interfaces exposed by the view manager, this object provides the
// canonical implementation of said interfaces but will call out to the wrapped
// instances.
class WindowManagerApp : public ApplicationDelegate,
                         public ViewManagerDelegate,
                         public ViewObserver,
                         public ui::EventHandler,
                         public FocusControllerObserver,
                         public InterfaceFactory<WindowManagerInternal> {
 public:
  WindowManagerApp(ViewManagerDelegate* view_manager_delegate,
                   WindowManagerDelegate* window_manager_delegate);
  ~WindowManagerApp() override;

  mojo::ViewEventDispatcher* event_dispatcher() {
    return view_event_dispatcher_.get();
  }

  // Register/deregister new connections to the window manager service.
  void AddConnection(WindowManagerImpl* connection);
  void RemoveConnection(WindowManagerImpl* connection);

  // These are canonical implementations of the window manager API methods.
  void SetCapture(Id view);
  void FocusWindow(Id view);
  void ActivateWindow(Id view);

  void DispatchInputEventToView(View* view, EventPtr event);
  void SetViewportSize(const gfx::Size& size);

  bool IsReady() const;

  FocusController* focus_controller() { return focus_controller_.get(); }

  void InitFocus(scoped_ptr<FocusRules> rules);

  // WindowManagerImpl::Embed() forwards to this. If connected to ViewManager
  // then forwards to delegate, otherwise waits for connection to establish then
  // forwards.
  void Embed(const String& url,
             InterfaceRequest<ServiceProvider> service_provider);

  // Overridden from ApplicationDelegate:
  void Initialize(ApplicationImpl* impl) override;
  bool ConfigureIncomingConnection(ApplicationConnection* connection) override;

 private:
  // TODO(sky): rename this. Connections is ambiguous.
  typedef std::set<WindowManagerImpl*> Connections;
  typedef std::set<Id> RegisteredViewIdSet;

  struct PendingEmbed;
  class WindowManagerInternalImpl;

  // Creates an ViewTarget for every view in the hierarchy beneath |view|,
  // and adds to the registry so that it can be retrieved later via
  // GetViewTargetForViewId().
  // TODO(beng): perhaps View should have a property bag.
  void RegisterSubtree(View* view);

  // Recursively invokes Unregister() for |view| and all its descendants.
  void UnregisterSubtree(View* view);

  // Deletes the ViewTarget associated with the hierarchy beneath |id|,
  // and removes from the registry.
  void Unregister(View* view);

  // Overridden from ViewManagerDelegate:
  void OnEmbed(ViewManager* view_manager,
               View* root,
               ServiceProviderImpl* exported_services,
               scoped_ptr<ServiceProvider> imported_services) override;
  void OnViewManagerDisconnected(ViewManager* view_manager) override;

  // Overridden from ViewObserver:
  void OnTreeChanged(const ViewObserver::TreeChangeParams& params) override;
  void OnViewDestroying(View* view) override;

  // Overridden from ui::EventHandler:
  void OnEvent(ui::Event* event) override;

  // Overridden from mojo::FocusControllerObserver:
  void OnViewFocused(View* gained_focus, View* lost_focus) override;
  void OnViewActivated(View* gained_active, View* lost_active) override;

  // Creates the connection to the ViewManager.
  void LaunchViewManager(ApplicationImpl* app);

  // InterfaceFactory<WindowManagerInternal>:
  void Create(ApplicationConnection* connection,
              InterfaceRequest<WindowManagerInternal> request) override;

  Shell* shell_;

  InterfaceFactoryImplWithContext<WindowManagerImpl, WindowManagerApp>
      window_manager_factory_;

  InterfaceFactoryImplWithContext<NativeViewportEventDispatcherImpl,
                                  WindowManagerApp>
      native_viewport_event_dispatcher_factory_;

  ViewManagerDelegate* wrapped_view_manager_delegate_;
  WindowManagerDelegate* window_manager_delegate_;

  ViewManager* view_manager_;
  scoped_ptr<ViewManagerClientFactory> view_manager_client_factory_;
  View* root_;

  scoped_ptr<mojo::FocusController> focus_controller_;

  Connections connections_;
  RegisteredViewIdSet registered_view_id_set_;

  WindowManagerInternalClientPtr window_manager_client_;

  ScopedVector<PendingEmbed> pending_embeds_;

  scoped_ptr<ViewManagerClient> view_manager_client_;

  scoped_ptr<ViewEventDispatcher> view_event_dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerApp);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_WINDOW_MANAGER_WINDOW_MANAGER_APP_H_
