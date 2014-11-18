// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/gles2/gpu_impl.h"

#include "gpu/command_buffer/service/mailbox_manager.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/services/gles2/command_buffer_impl.h"
#include "ui/gl/gl_share_group.h"

namespace mojo {

GpuImpl::GpuImpl(
    InterfaceRequest<Gpu> request,
    const scoped_refptr<gfx::GLShareGroup>& share_group,
    const scoped_refptr<gpu::gles2::MailboxManager> mailbox_manager)
    : share_group_(share_group),
      mailbox_manager_(mailbox_manager),
      binding_(this, request.Pass()) {
}

GpuImpl::~GpuImpl() {
}

void GpuImpl::CreateOnscreenGLES2Context(
    uint64_t native_viewport_id,
    SizePtr size,
    InterfaceRequest<CommandBuffer> request) {
  gfx::AcceleratedWidget widget = bit_cast<gfx::AcceleratedWidget>(
      static_cast<uintptr_t>(native_viewport_id));
  new CommandBufferImpl(request.Pass(), widget, size.To<gfx::Size>(),
                        share_group_.get(), mailbox_manager_.get());
}

void GpuImpl::CreateOffscreenGLES2Context(
    InterfaceRequest<CommandBuffer> request) {
  new CommandBufferImpl(request.Pass(), share_group_.get(),
                        mailbox_manager_.get());
}

}  // namespace mojo
