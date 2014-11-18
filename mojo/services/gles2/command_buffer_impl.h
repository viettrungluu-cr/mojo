// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_SERVICES_GLES2_COMMAND_BUFFER_IMPL_H_
#define MOJO_SERVICES_GLES2_COMMAND_BUFFER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "mojo/services/public/interfaces/gpu/command_buffer.mojom.h"

namespace mojo {
class CommandBufferDriver;

class CommandBufferImpl : public CommandBuffer {
 public:
  CommandBufferImpl(InterfaceRequest<CommandBuffer> request,
                    scoped_ptr<CommandBufferDriver> driver);
  ~CommandBufferImpl() override;

  void Initialize(CommandBufferSyncClientPtr sync_client,
                  mojo::ScopedSharedBufferHandle shared_state) override;
  void SetGetBuffer(int32_t buffer) override;
  void Flush(int32_t put_offset) override;
  void MakeProgress(int32_t last_get_offset) override;
  void RegisterTransferBuffer(int32_t id,
                              mojo::ScopedSharedBufferHandle transfer_buffer,
                              uint32_t size) override;
  void DestroyTransferBuffer(int32_t id) override;
  void Echo(const Callback<void()>& callback) override;

 private:
  void OnContextLost(int32_t reason);

  StrongBinding<CommandBuffer> binding_;
  scoped_ptr<CommandBufferDriver> driver_;

  DISALLOW_COPY_AND_ASSIGN(CommandBufferImpl);
};

}  // namespace mojo

#endif  // MOJO_SERVICES_GLES2_COMMAND_BUFFER_IMPL_H_
