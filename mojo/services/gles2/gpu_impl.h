// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "mojo/public/cpp/bindings/interface_request.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/services/public/interfaces/geometry/geometry.mojom.h"
#include "mojo/services/public/interfaces/gpu/command_buffer.mojom.h"
#include "mojo/services/public/interfaces/gpu/gpu.mojom.h"

namespace gfx {
class GLShareGroup;
}

namespace gpu {
namespace gles2 {
class MailboxManager;
}
}

namespace mojo {

class GpuImpl : public Gpu {
 public:
  GpuImpl(InterfaceRequest<Gpu> request,
          const scoped_refptr<gfx::GLShareGroup>& share_group,
          const scoped_refptr<gpu::gles2::MailboxManager> mailbox_manager);

  ~GpuImpl() override;

  void CreateOnscreenGLES2Context(
      uint64_t native_viewport_id,
      SizePtr size,
      InterfaceRequest<CommandBuffer> command_buffer_request) override;

  void CreateOffscreenGLES2Context(
      InterfaceRequest<CommandBuffer> command_buffer_request) override;

 private:
  // We need to share these across all CommandBuffer instances so that contexts
  // they create can share resources with each other via mailboxes.
  scoped_refptr<gfx::GLShareGroup> share_group_;
  scoped_refptr<gpu::gles2::MailboxManager> mailbox_manager_;

  StrongBinding<Gpu> binding_;

  DISALLOW_COPY_AND_ASSIGN(GpuImpl);
};

}  // namespace mojo
