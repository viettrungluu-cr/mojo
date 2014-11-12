// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EDK_EMBEDDER_ENTRYPOINTS_H_
#define MOJO_EDK_EMBEDDER_ENTRYPOINTS_H_

namespace mojo {

namespace system {
class Core;
}  // namespace system

namespace embedder {
namespace internal {

// Sets the instance of |Core| used by the system functions.
void SetCore(system::Core* core);

// Gets the instance of |Core| used by the system functions.
system::Core* GetCore();

}  // namespace internal
}  // namepace embedder

}  // namespace mojo

#endif  // MOJO_EDK_EMBEDDER_ENTRYPOINTS_H_
