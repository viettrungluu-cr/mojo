// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/mojo_gl_context.h"

#include "base/logging.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/interfaces/application/shell.mojom.h"

namespace mojo {
namespace examples {
namespace {

void OnContextLost(void* unused) {
  LOG(FATAL) << "Context lost.";
}
}

MojoGLContext::MojoGLContext(Shell* shell) {
  ServiceProviderPtr native_viewport;
  shell->ConnectToApplication("mojo:native_viewport_service",
                              GetProxy(&native_viewport));
  ConnectToService(native_viewport.get(), &gpu_service_);
  CommandBufferPtr command_buffer;
  gpu_service_->CreateOffscreenGLES2Context(GetProxy(&command_buffer));
  context_ = MojoGLES2CreateContext(
      command_buffer.PassMessagePipe().release().value(), &OnContextLost, 0,
      Environment::GetDefaultAsyncWaiter());
  MojoGLES2MakeCurrent(context_);
}

MojoGLContext::~MojoGLContext() {
  MojoGLES2DestroyContext(context_);
}

}  // namespace examples
}  // namespace mojo
