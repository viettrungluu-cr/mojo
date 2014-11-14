// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/ganesh_context.h"

#include "base/logging.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/client/gles2_lib.h"
#include "gpu/skia_bindings/gl_bindings_skia_cmd_buffer.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace examples {
namespace {

// The limit of the number of GPU resources we hold in the GrContext's
// GPU cache.
const int kMaxGaneshResourceCacheCount = 2048;

// The limit of the bytes allocated toward GPU resources in the GrContext's
// GPU cache.
const size_t kMaxGaneshResourceCacheBytes = 96 * 1024 * 1024;
}

GaneshContext::GaneshContext(MojoGLES2Context gl_context) {
  gpu::gles2::GLES2Interface* gl = static_cast<gpu::gles2::GLES2Interface*>(
      MojoGLES2GetGLES2Interface(gl_context));
  gles2::SetGLContext(gl);

  skia::RefPtr<GrGLInterface> interface =
      skia::AdoptRef(skia_bindings::CreateCommandBufferSkiaGLBinding());
  DCHECK(interface);

  context_ = skia::AdoptRef(GrContext::Create(
      kOpenGL_GrBackend, reinterpret_cast<GrBackendContext>(interface.get())));
  DCHECK(context_);
  context_->setResourceCacheLimits(kMaxGaneshResourceCacheCount,
                                   kMaxGaneshResourceCacheBytes);
}

GaneshContext::~GaneshContext() {
  gles2::SetGLContext(nullptr);
}

}  // namespace examples
