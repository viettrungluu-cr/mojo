// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GANESH_APP_GANESH_TEXTURE_H_
#define EXAMPLES_GANESH_APP_GANESH_TEXTURE_H_

#include "mojo/services/public/interfaces/geometry/geometry.mojom.h"
#include "skia/ext/refptr.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace examples {

class GaneshTexture {
 public:
  GaneshTexture(GrContext* gr_context, const mojo::Size& size);
  ~GaneshTexture();

  uint32_t texture_id() const { return texture_id_; }
  SkCanvas* canvas() const { return surface_->getCanvas(); }

 private:
  uint32_t texture_id_;
  skia::RefPtr<SkSurface> surface_;
};

}  // namespace examples

#endif  // EXAMPLES_GANESH_APP_GANESH_TEXTURE_H_
