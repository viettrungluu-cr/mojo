// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/edk/system/configuration.h"

namespace mojo {
namespace system {
namespace internal {
// These default values should be synced with the documentation in
// mojo/edk/embedder/configuration.h.
embedder::Configuration g_configuration = {
    // max_handle_table_size
    1000000,
    // max_mapping_table_sze
    1000000,
    // max_wait_many_num_handles
    1000000,
    // max_message_num_bytes
    4 * 1024 * 1024,
    // max_message_num_handles
    10000,
    // max_data_pipe_capacity_bytes
    256 * 1024 * 1024,
    // default_data_pipe_capacity_bytes
    1024 * 1024,
    // data_pipe_buffer_alignment_bytes
    16,
    // max_shared_memory_num_bytes
    1024 * 1024 * 1024};

}  // namespace internal
}  // namespace system
}  // namespace mojo
