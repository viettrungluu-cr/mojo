// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/services/gles2/command_buffer_impl.h"

#include "mojo/services/gles2/command_buffer_driver.h"

namespace mojo {

CommandBufferImpl::CommandBufferImpl(InterfaceRequest<CommandBuffer> request,
                                     scoped_ptr<CommandBufferDriver> driver)
    : binding_(this, request.Pass()), driver_(driver.Pass()) {
  driver_->SetContextLostCallback(
      base::Bind(&CommandBufferImpl::OnContextLost, base::Unretained(this)));
}

CommandBufferImpl::~CommandBufferImpl() {
  binding_.client()->DidDestroy();
}

void CommandBufferImpl::Initialize(
    CommandBufferSyncClientPtr sync_client,
    mojo::ScopedSharedBufferHandle shared_state) {
  driver_->Initialize(sync_client.Pass(), shared_state.Pass());
}

void CommandBufferImpl::SetGetBuffer(int32_t buffer) {
  driver_->SetGetBuffer(buffer);
}

void CommandBufferImpl::Flush(int32_t put_offset) {
  driver_->Flush(put_offset);
}

void CommandBufferImpl::MakeProgress(int32_t last_get_offset) {
  driver_->MakeProgress(last_get_offset);
}

void CommandBufferImpl::RegisterTransferBuffer(
    int32_t id,
    mojo::ScopedSharedBufferHandle transfer_buffer,
    uint32_t size) {
  driver_->RegisterTransferBuffer(id, transfer_buffer.Pass(), size);
}

void CommandBufferImpl::DestroyTransferBuffer(int32_t id) {
  driver_->DestroyTransferBuffer(id);
}

void CommandBufferImpl::Echo(const Callback<void()>& callback) {
  callback.Run();
}

void CommandBufferImpl::OnContextLost(int32_t reason) {
  binding_.client()->LostContext(reason);
}

}  // namespace mojo
