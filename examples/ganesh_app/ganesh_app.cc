// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "base/threading/platform_thread.h"
#include "examples/ganesh_app/painter.h"
#include "examples/surfaces_app/child.mojom.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/bindings/string.h"
#include "mojo/services/public/interfaces/gpu/gpu.mojom.h"

namespace mojo {
namespace examples {

class GaneshApp : public ApplicationDelegate, public InterfaceFactory<Child> {
 public:
  GaneshApp() {}
  virtual ~GaneshApp() {}

  virtual void Initialize(ApplicationImpl* app) override {
    surfaces_service_connection_ =
        app->ConnectToApplication("mojo:surfaces_service");
    // TODO(jamesr): Should be mojo:gpu_service
    app->ConnectToService("mojo:native_viewport_service", &gpu_service_);
  }

  // ApplicationDelegate implementation.
  virtual bool ConfigureIncomingConnection(
      ApplicationConnection* connection) override {
    connection->AddService(this);
    return true;
  }

  // InterfaceFactory<Child> implementation.
  virtual void Create(ApplicationConnection* connection,
                      InterfaceRequest<Child> request) override {
    CommandBufferPtr command_buffer;
    gpu_service_->CreateOffscreenGLES2Context(GetProxy(&command_buffer));
    BindToRequest(
        new Painter(surfaces_service_connection_, command_buffer.Pass()),
        &request);
  }

 private:
  ApplicationConnection* surfaces_service_connection_;
  GpuPtr gpu_service_;

  DISALLOW_COPY_AND_ASSIGN(GaneshApp);
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::examples::GaneshApp);
  return runner.Run(shell_handle);
}
