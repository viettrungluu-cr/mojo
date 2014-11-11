// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/window_manager/window_manager_app.h"

#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/converters/input_events/input_events_type_converters.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/public/cpp/view_manager/view_manager.h"
#include "mojo/services/window_manager/focus_controller.h"
#include "mojo/services/window_manager/focus_rules.h"
#include "mojo/services/window_manager/view_event_dispatcher.h"
#include "mojo/services/window_manager/view_target.h"
#include "mojo/services/window_manager/view_targeter.h"
#include "mojo/services/window_manager/window_manager_delegate.h"
#include "ui/base/hit_test.h"

namespace mojo {

namespace {

Id GetIdForView(View* view) {
  return view ? view->id() : 0;
}

}  // namespace

class WindowManagerApp::WindowManagerInternalImpl
    : public InterfaceImpl<WindowManagerInternal> {
 public:
  WindowManagerInternalImpl(WindowManagerApp* app) : app_(app) {}
  ~WindowManagerInternalImpl() override {}

  // WindowManagerInternal:
  void CreateWindowManagerForViewManagerClient(
      uint16_t connection_id,
      ScopedMessagePipeHandle window_manager_pipe) override {
    // |wm_internal| is tied to the life of the pipe.
    WindowManagerImpl* wm = new WindowManagerImpl(app_, true);
    WeakBindToPipe(wm, window_manager_pipe.Pass());
  }

  // InterfaceImpl:
  void OnConnectionError() override {
    // Necessary since we used WeakBindToPipe and are not automatically deleted.
    delete this;
  }

 private:
  WindowManagerApp* app_;

  DISALLOW_COPY_AND_ASSIGN(WindowManagerInternalImpl);
};

// Used for calls to Embed() that occur before we've connected to the
// ViewManager.
struct WindowManagerApp::PendingEmbed {
  String url;
  InterfaceRequest<mojo::ServiceProvider> service_provider;
};

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, public:

WindowManagerApp::WindowManagerApp(
    ViewManagerDelegate* view_manager_delegate,
    WindowManagerDelegate* window_manager_delegate)
    : shell_(nullptr),
      window_manager_factory_(this),
      native_viewport_event_dispatcher_factory_(this),
      wrapped_view_manager_delegate_(view_manager_delegate),
      window_manager_delegate_(window_manager_delegate),
      view_manager_(nullptr),
      root_(nullptr) {
}

WindowManagerApp::~WindowManagerApp() {
  STLDeleteElements(&connections_);
}

// static
View* WindowManagerApp::GetViewForViewTarget(ViewTarget* target) {
  return target->view();
}

ViewTarget* WindowManagerApp::GetViewTargetForViewId(Id view) {
  ViewIdToViewTargetMap::const_iterator it =
      view_id_to_view_target_map_.find(view);
  return it != view_id_to_view_target_map_.end() ? it->second : nullptr;
}

void WindowManagerApp::AddConnection(WindowManagerImpl* connection) {
  DCHECK(connections_.find(connection) == connections_.end());
  connections_.insert(connection);
}

void WindowManagerApp::RemoveConnection(WindowManagerImpl* connection) {
  DCHECK(connections_.find(connection) != connections_.end());
  connections_.erase(connection);
}

void WindowManagerApp::SetCapture(Id view) {
  // TODO(erg): Capture. Another pile of worms that is mixed in here.

  // capture_client_->capture_client()->SetCapture(GetWindowForViewId(view));

  // TODO(beng): notify connected clients that capture has changed, probably
  //             by implementing some capture-client observer.
}

void WindowManagerApp::FocusWindow(Id view_id) {
  View* view = view_manager_->GetViewById(view_id);
  DCHECK(view);
  focus_controller_->FocusView(view);
}

void WindowManagerApp::ActivateWindow(Id view_id) {
  View* view = view_manager_->GetViewById(view_id);
  DCHECK(view);
  focus_controller_->ActivateView(view);
}

bool WindowManagerApp::IsReady() const {
  return view_manager_ && root_;
}

void WindowManagerApp::InitFocus(scoped_ptr<mojo::FocusRules> rules) {
  focus_controller_.reset(new mojo::FocusController(rules.Pass()));
  focus_controller_->AddObserver(this);
}

void WindowManagerApp::Embed(
    const String& url,
    InterfaceRequest<ServiceProvider> service_provider) {
  if (view_manager_) {
    window_manager_delegate_->Embed(url, service_provider.Pass());
    return;
  }
  scoped_ptr<PendingEmbed> pending_embed(new PendingEmbed);
  pending_embed->url = url;
  pending_embed->service_provider = service_provider.Pass();
  pending_embeds_.push_back(pending_embed.release());
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, ApplicationDelegate implementation:

void WindowManagerApp::Initialize(ApplicationImpl* impl) {
  shell_ = impl->shell();
  LaunchViewManager(impl);
}

bool WindowManagerApp::ConfigureIncomingConnection(
    ApplicationConnection* connection) {
  connection->AddService(&window_manager_factory_);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, ViewManagerDelegate implementation:

void WindowManagerApp::OnEmbed(ViewManager* view_manager,
                               View* root,
                               ServiceProviderImpl* exported_services,
                               scoped_ptr<ServiceProvider> imported_services) {
  DCHECK(!view_manager_ && !root_);
  view_manager_ = view_manager;
  root_ = root;

  view_event_dispatcher_.reset(new ViewEventDispatcher());

  RegisterSubtree(root_, nullptr);

  // TODO(erg): Also move the capture client over.
  //
  // capture_client_.reset(
  //     new wm::ScopedCaptureClient(window_tree_host_->window()));

  if (wrapped_view_manager_delegate_) {
    wrapped_view_manager_delegate_->OnEmbed(
        view_manager, root, exported_services, imported_services.Pass());
  }

  for (PendingEmbed* pending_embed : pending_embeds_)
    Embed(pending_embed->url, pending_embed->service_provider.Pass());
  pending_embeds_.clear();
}

void WindowManagerApp::OnViewManagerDisconnected(
    ViewManager* view_manager) {
  DCHECK_EQ(view_manager_, view_manager);
  if (wrapped_view_manager_delegate_)
    wrapped_view_manager_delegate_->OnViewManagerDisconnected(view_manager);
  view_manager_ = nullptr;
  base::MessageLoop::current()->Quit();
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, ViewObserver implementation:

void WindowManagerApp::OnTreeChanged(
    const ViewObserver::TreeChangeParams& params) {
  if (params.receiver != root_)
    return;
  DCHECK(params.old_parent || params.new_parent);
  if (!params.target)
    return;

  if (params.new_parent) {
    if (view_id_to_view_target_map_.find(params.target->id()) ==
        view_id_to_view_target_map_.end()) {
      ViewIdToViewTargetMap::const_iterator it =
          view_id_to_view_target_map_.find(params.new_parent->id());
      DCHECK(it != view_id_to_view_target_map_.end());
      RegisterSubtree(params.target, it->second);
    }
  } else if (params.old_parent) {
    UnregisterSubtree(params.target);
  }
}

void WindowManagerApp::OnViewDestroying(View* view) {
  if (view != root_) {
    Unregister(view);
    return;
  }
  root_ = nullptr;
  if (focus_controller_)
    focus_controller_->RemoveObserver(this);
}

void WindowManagerApp::OnViewBoundsChanged(View* view,
                                           const Rect& old_bounds,
                                           const Rect& new_bounds) {
  // aura::Window* window = GetWindowForViewId(view->id());
  // window->SetBounds(new_bounds.To<gfx::Rect>());
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, ui::EventHandler implementation:

void WindowManagerApp::OnEvent(ui::Event* event) {
  if (!window_manager_client_)
    return;

  View* view = GetViewForViewTarget(static_cast<ViewTarget*>(event->target()));
  if (!view)
    return;

  if (focus_controller_)
    focus_controller_->OnEvent(event);

  window_manager_client_->DispatchInputEventToView(view->id(),
                                                   Event::From(*event));
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, mojo::FocusControllerObserver implementation:

void WindowManagerApp::OnViewFocused(View* gained_focus,
                                     View* lost_focus) {
  for (Connections::const_iterator it = connections_.begin();
       it != connections_.end(); ++it) {
    (*it)->NotifyViewFocused(GetIdForView(gained_focus),
                             GetIdForView(lost_focus));
  }
}

void WindowManagerApp::OnViewActivated(View* gained_active,
                                       View* lost_active) {
  for (Connections::const_iterator it = connections_.begin();
       it != connections_.end(); ++it) {
    (*it)->NotifyWindowActivated(GetIdForView(gained_active),
                                 GetIdForView(lost_active));
  }
  if (gained_active)
    gained_active->MoveToFront();
}

////////////////////////////////////////////////////////////////////////////////
// WindowManagerApp, private:

void WindowManagerApp::RegisterSubtree(View* view, ViewTarget* parent) {
  view->AddObserver(this);
  DCHECK(view_id_to_view_target_map_.find(view->id()) ==
         view_id_to_view_target_map_.end());
  ViewTarget* target = new ViewTarget(this, view);
  // All events pass through the root during dispatch, so we only need a handler
  // installed there.
  if (view == root_) {
    target->SetEventTargeter(scoped_ptr<ViewTargeter>(new ViewTargeter()));
    target->AddPreTargetHandler(this);
    view_event_dispatcher_->SetRootViewTarget(target);
  }
  // TODO(erg): Why is there no matching RemoveChild()? How does that work in
  // the aura version?
  view_id_to_view_target_map_[view->id()] = target;
  View::Children::const_iterator it = view->children().begin();
  for (; it != view->children().end(); ++it)
    RegisterSubtree(*it, target);
}

void WindowManagerApp::UnregisterSubtree(View* view) {
  for (View* child : view->children())
    UnregisterSubtree(child);
  Unregister(view);
}

void WindowManagerApp::Unregister(View* view) {
  ViewIdToViewTargetMap::iterator it =
      view_id_to_view_target_map_.find(view->id());
  if (it == view_id_to_view_target_map_.end()) {
    // Because we unregister in OnViewDestroying() we can still get a subsequent
    // OnTreeChanged for the same view. Ignore this one.
    return;
  }
  view->RemoveObserver(this);
  DCHECK(it != view_id_to_view_target_map_.end());
  // Delete before we remove from map as destruction may want to look up view
  // for window.
  delete it->second;
  view_id_to_view_target_map_.erase(it);
}

void WindowManagerApp::SetViewportSize(const gfx::Size& size) {
  window_manager_client_->SetViewportSize(Size::From(size));
}

void WindowManagerApp::LaunchViewManager(ApplicationImpl* app) {
  // TODO(sky): figure out logic if this connection goes away.
  view_manager_client_factory_.reset(
      new ViewManagerClientFactory(shell_, this));

  MessagePipe pipe;
  ApplicationConnection* view_manager_app =
      app->ConnectToApplication("mojo:view_manager");
  ServiceProvider* view_manager_service_provider =
      view_manager_app->GetServiceProvider();
  view_manager_service_provider->ConnectToService(ViewManagerService::Name_,
                                                  pipe.handle1.Pass());
  view_manager_client_ = ViewManagerClientFactory::WeakBindViewManagerToPipe(
                             pipe.handle0.Pass(), shell_, this).Pass();

  view_manager_app->AddService(&native_viewport_event_dispatcher_factory_);
  view_manager_app->AddService(this);

  view_manager_app->ConnectToService(&window_manager_client_);
}

void WindowManagerApp::Create(ApplicationConnection* connection,
                              InterfaceRequest<WindowManagerInternal> request) {
  WindowManagerInternalImpl* impl = new WindowManagerInternalImpl(this);
  BindToRequest(impl, &request);
}

}  // namespace mojo
