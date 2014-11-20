// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/services/gles2/gpu_impl.h"
#include "mojo/services/native_viewport/native_viewport_impl.h"
#include "mojo/services/public/cpp/native_viewport/args.h"
#include "ui/gl/gl_surface.h"

namespace mojo {

class NativeViewportAppDelegate
    : public ApplicationDelegate,
      public InterfaceFactory<NativeViewport>,
      public InterfaceFactory<Gpu> {
 public:
  NativeViewportAppDelegate() : is_headless_(false) {}
  ~NativeViewportAppDelegate() override {}

 private:
  // ApplicationDelegate implementation.
  void Initialize(ApplicationImpl* application) override {
    app_ = application;

    if (app_->HasArg(kUseTestConfig))
      gfx::GLSurface::InitializeOneOffForTests();
    else if (app_->HasArg(kUseOSMesa))
      gfx::GLSurface::InitializeOneOff(gfx::kGLImplementationOSMesaGL);
    else
      gfx::GLSurface::InitializeOneOff();

    is_headless_ = app_->HasArg(kUseHeadlessConfig);
  }

  bool ConfigureIncomingConnection(
      mojo::ApplicationConnection* connection) override {
    connection->AddService<NativeViewport>(this);
    connection->AddService<Gpu>(this);
    return true;
  }

  // InterfaceFactory<NativeViewport> implementation.
  void Create(ApplicationConnection* connection,
              InterfaceRequest<NativeViewport> request) override {
    BindToRequest(new NativeViewportImpl(app_, is_headless_), &request);
  }

  // InterfaceFactory<Gpu> implementation.
  void Create(ApplicationConnection* connection,
              InterfaceRequest<Gpu> request) override {
    if (!gpu_state_.get())
      gpu_state_ = new GpuImpl::State;
    new GpuImpl(request.Pass(), gpu_state_);
  }

  ApplicationImpl* app_;
  scoped_refptr<GpuImpl::State> gpu_state_;
  bool is_headless_;
  DISALLOW_COPY_AND_ASSIGN(NativeViewportAppDelegate);
};
}

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::NativeViewportAppDelegate);
  runner.set_message_loop_type(base::MessageLoop::TYPE_UI);
  return runner.Run(shell_handle);
}
