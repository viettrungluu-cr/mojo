// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/mojo_gl_context.h"

#include "base/logging.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/interfaces/application/shell.mojom.h"

namespace examples {
namespace {

void OnContextLost(void* unused) {
  LOG(FATAL) << "Context lost.";
}
}

MojoGLContext::MojoGLContext(mojo::Shell* shell) {
  mojo::ServiceProviderPtr native_viewport;
  shell->ConnectToApplication("mojo:native_viewport_service",
                              mojo::GetProxy(&native_viewport));
  mojo::ConnectToService(native_viewport.get(), &gpu_service_);
  mojo::CommandBufferPtr command_buffer;
  gpu_service_->CreateOffscreenGLES2Context(mojo::GetProxy(&command_buffer));
  context_ = MojoGLES2CreateContext(
      command_buffer.PassMessagePipe().release().value(), &OnContextLost, 0,
      mojo::Environment::GetDefaultAsyncWaiter());
  MojoGLES2MakeCurrent(context_);
}

MojoGLContext::~MojoGLContext() {
  MojoGLES2DestroyContext(context_);
}

}  // namespace examples
