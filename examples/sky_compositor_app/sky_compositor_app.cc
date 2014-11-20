// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/public/cpp/view_manager/view_manager.h"
#include "mojo/services/public/cpp/view_manager/view_manager_client_factory.h"
#include "mojo/services/public/cpp/view_manager/view_manager_delegate.h"
#include "sky/compositor/layer.h"
#include "sky/compositor/layer_host.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace examples {
namespace {

gfx::Size ToSize(const mojo::Rect& rect) {
  return gfx::Size(rect.width, rect.height);
}
}

class SkyCompositorApp : public mojo::ApplicationDelegate,
                         public mojo::ViewManagerDelegate,
                         public sky::LayerClient,
                         public sky::LayerHostClient {
 public:
  SkyCompositorApp() : shell_(nullptr), view_(nullptr) {}
  virtual ~SkyCompositorApp() {}

  void Initialize(mojo::ApplicationImpl* app) override {
    shell_ = app->shell();
    view_manager_client_factory_.reset(
        new mojo::ViewManagerClientFactory(app->shell(), this));
  }

  bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override {
    connection->AddService(view_manager_client_factory_.get());
    return true;
  }

  void OnEmbed(mojo::ViewManager* view_manager,
               mojo::View* root,
               mojo::ServiceProviderImpl* exported_services,
               scoped_ptr<mojo::ServiceProvider> imported_services) override {
    view_ = root;

    layer_host_.reset(new sky::LayerHost(this));
    root_layer_ = make_scoped_refptr(new sky::Layer(this));
    layer_host_->SetRootLayer(root_layer_);
  }

  void OnViewManagerDisconnected(mojo::ViewManager* view_manager) override {
    mojo::ApplicationImpl::Terminate();
  }

 private:
  // sky::LayerHostClient
  mojo::Shell* GetShell() override { return shell_; }

  void BeginFrame() override { root_layer_->SetSize(ToSize(view_->bounds())); }

  void OnSurfaceIdAvailable(mojo::SurfaceIdPtr surface_id) override {
    view_->SetSurfaceId(surface_id.Pass());
  }

  // sky::LayerClient
  void PaintContents(SkCanvas* canvas, const gfx::Rect& clip) override {
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    paint.setFlags(SkPaint::kAntiAlias_Flag);
    canvas->drawCircle(50, 100, 100, paint);
  }

  mojo::Shell* shell_;
  scoped_ptr<mojo::ViewManagerClientFactory> view_manager_client_factory_;
  mojo::View* view_;

  scoped_ptr<sky::LayerHost> layer_host_;
  scoped_refptr<sky::Layer> root_layer_;

  DISALLOW_COPY_AND_ASSIGN(SkyCompositorApp);
};

}  // namespace examples

MojoResult MojoMain(MojoHandle handle) {
  mojo::ApplicationRunnerChromium runner(new examples::SkyCompositorApp);
  return runner.Run(handle);
}
