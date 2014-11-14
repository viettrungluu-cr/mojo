// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/ganesh_texture.h"

#include "base/logging.h"
#include "mojo/public/c/gles2/gles2.h"

namespace mojo {
namespace examples {

GaneshTexture::GaneshTexture(GrContext* gr_context, const Size& size)
    : texture_id_(0u) {
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  GrBackendTextureDesc desc;
  desc.fFlags = kRenderTarget_GrBackendTextureFlag;
  desc.fWidth = size.width;
  desc.fHeight = size.height;
  desc.fConfig = kSkia8888_GrPixelConfig;
  desc.fOrigin = kTopLeft_GrSurfaceOrigin;
  desc.fTextureHandle = texture_id_;

  auto texture = skia::AdoptRef(gr_context->wrapBackendTexture(desc));
  DCHECK(texture);
  surface_ = skia::AdoptRef(
      SkSurface::NewRenderTargetDirect(texture->asRenderTarget()));
  DCHECK(surface_);
}

GaneshTexture::~GaneshTexture() {
}

}  // namespace examples
}  // namespace mojo
