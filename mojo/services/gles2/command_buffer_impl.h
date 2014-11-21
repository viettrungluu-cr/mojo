// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_GLES2_COMMAND_BUFFER_IMPL_H_
#define MOJO_SERVICES_GLES2_COMMAND_BUFFER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/services/public/interfaces/gpu/command_buffer.mojom.h"

namespace gpu {
class SyncPointManager;
}

namespace mojo {
class CommandBufferDriver;

// This class listens to the CommandBuffer message pipe on a low-latency thread
// so that we can insert sync points without blocking on the GL driver. It
// forwards most method calls to the CommandBufferDriver, which runs on the
// same thread as the native viewport.
class CommandBufferImpl : public CommandBuffer {
 public:
  CommandBufferImpl(
      InterfaceRequest<CommandBuffer> request,
      scoped_refptr<base::SingleThreadTaskRunner> control_task_runner,
      gpu::SyncPointManager* sync_point_manager,
      scoped_ptr<CommandBufferDriver> driver);
  ~CommandBufferImpl() override;

  void Initialize(CommandBufferSyncClientPtr sync_client,
                  CommandBufferSyncPointClientPtr sync_point_client,
                  ScopedSharedBufferHandle shared_state) override;
  void SetGetBuffer(int32_t buffer) override;
  void Flush(int32_t put_offset) override;
  void MakeProgress(int32_t last_get_offset) override;
  void RegisterTransferBuffer(int32_t id,
                              ScopedSharedBufferHandle transfer_buffer,
                              uint32_t size) override;
  void DestroyTransferBuffer(int32_t id) override;
  void InsertSyncPoint(bool retire) override;
  void RetireSyncPoint(uint32_t sync_point) override;
  void Echo(const Callback<void()>& callback) override;

 private:
  void BindToRequest(InterfaceRequest<CommandBuffer> request);
  void OnContextLost(int32_t reason);

  scoped_refptr<gpu::SyncPointManager> sync_point_manager_;
  scoped_refptr<base::SingleThreadTaskRunner> driver_task_runner_;
  scoped_ptr<CommandBufferDriver> driver_;
  CommandBufferSyncPointClientPtr sync_point_client_;
  StrongBinding<CommandBuffer> binding_;

  base::WeakPtrFactory<CommandBufferImpl> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(CommandBufferImpl);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_GLES2_COMMAND_BUFFER_IMPL_H_
