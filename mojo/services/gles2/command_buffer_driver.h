// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_GLES2_COMMAND_BUFFER_DRIVER_H_
#define MOJO_SERVICES_GLES2_COMMAND_BUFFER_DRIVER_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/timer/timer.h"
#include "mojo/services/public/interfaces/gpu/command_buffer.mojom.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"

namespace gpu {
class CommandBufferService;
class GpuScheduler;
class GpuControlService;
namespace gles2 {
class GLES2Decoder;
class MailboxManager;
}
}

namespace gfx {
class GLContext;
class GLShareGroup;
class GLSurface;
}

namespace mojo {

class CommandBufferDriver {
 public:
  // Offscreen.
  CommandBufferDriver(gfx::GLShareGroup* share_group,
                      gpu::gles2::MailboxManager* mailbox_manager);
  // Onscreen.
  CommandBufferDriver(gfx::AcceleratedWidget widget,
                      const gfx::Size& size,
                      gfx::GLShareGroup* share_group,
                      gpu::gles2::MailboxManager* mailbox_manager);
  ~CommandBufferDriver();

  void Initialize(CommandBufferSyncClientPtr sync_client,
                  ScopedSharedBufferHandle shared_state);
  void SetGetBuffer(int32_t buffer);
  void Flush(int32_t put_offset);
  void MakeProgress(int32_t last_get_offset);
  void RegisterTransferBuffer(int32_t id,
                              ScopedSharedBufferHandle transfer_buffer,
                              uint32_t size);
  void DestroyTransferBuffer(int32_t id);

  void SetContextLostCallback(const base::Callback<void(int32_t)>& callback);

 private:
  bool DoInitialize(ScopedSharedBufferHandle shared_state);
  void OnResize(gfx::Size size, float scale_factor);
  void OnParseError();
  void OnContextLost(uint32_t reason);

  CommandBufferSyncClientPtr sync_client_;

  gfx::AcceleratedWidget widget_;
  gfx::Size size_;
  scoped_ptr<gpu::CommandBufferService> command_buffer_;
  scoped_ptr<gpu::gles2::GLES2Decoder> decoder_;
  scoped_ptr<gpu::GpuScheduler> scheduler_;
  scoped_refptr<gfx::GLContext> context_;
  scoped_refptr<gfx::GLSurface> surface_;
  scoped_refptr<gfx::GLShareGroup> share_group_;
  scoped_refptr<gpu::gles2::MailboxManager> mailbox_manager_;
  base::Callback<void(int32_t)> context_lost_callback_;

  DISALLOW_COPY_AND_ASSIGN(CommandBufferDriver);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_GLES2_COMMAND_BUFFER_DRIVER_H_
