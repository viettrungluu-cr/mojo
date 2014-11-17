// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/gpu/gl_context.h"

#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "mojo/services/public/interfaces/gpu/gpu.mojom.h"

namespace mojo {

GLContext::Observer::~Observer() {
}

GLContext::GLContext(mojo::Shell* shell) : weak_factory_(this) {
  mojo::ServiceProviderPtr native_viewport;
  shell->ConnectToApplication("mojo:native_viewport_service",
                              mojo::GetProxy(&native_viewport));
  mojo::GpuPtr gpu_service;
  mojo::ConnectToService(native_viewport.get(), &gpu_service);
  mojo::CommandBufferPtr command_buffer;
  gpu_service->CreateOffscreenGLES2Context(mojo::GetProxy(&command_buffer));
  context_ = MojoGLES2CreateContext(
      command_buffer.PassMessagePipe().release().value(), &ContextLostThunk,
      this, mojo::Environment::GetDefaultAsyncWaiter());
  gl_ = static_cast<gpu::gles2::GLES2Interface*>(
      MojoGLES2GetGLES2Interface(context_));
}

GLContext::~GLContext() {
  MojoGLES2DestroyContext(context_);
}

base::WeakPtr<GLContext> GLContext::Create(mojo::Shell* shell) {
  return (new GLContext(shell))->weak_factory_.GetWeakPtr();
}

void GLContext::MakeCurrent() {
  MojoGLES2MakeCurrent(context_);
}

void GLContext::Destroy() {
  delete this;
}

void GLContext::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void GLContext::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void GLContext::ContextLostThunk(void* self) {
  static_cast<GLContext*>(self)->OnContextLost();
}

void GLContext::OnContextLost() {
  FOR_EACH_OBSERVER(Observer, observers_, OnContextLost());
}

}  // namespace mojo
