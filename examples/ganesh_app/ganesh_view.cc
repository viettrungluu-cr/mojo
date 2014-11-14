// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/ganesh_view.h"

#include "examples/ganesh_app/ganesh_texture.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace mojo {
namespace examples {
namespace {

Size ToSize(const Rect& rect) {
  Size size;
  size.width = rect.width;
  size.height = rect.height;
  return size;
}
}

GaneshView::GaneshView(Shell* shell, View* view)
    : view_(view),
      gl_context_(shell),
      gr_context_(gl_context_.context()),
      texture_uploader_(this, shell, gl_context_.context()) {
  Draw(ToSize(view_->bounds()));
}

GaneshView::~GaneshView() {
}

void GaneshView::OnSurfaceIdAvailable(SurfaceIdPtr surface_id) {
  view_->SetSurfaceId(surface_id.Pass());
}

void GaneshView::OnViewDestroyed(View* view) {
  delete this;
}

void GaneshView::OnViewBoundsChanged(View* view,
                                     const Rect& old_bounds,
                                     const Rect& new_bounds) {
  Draw(ToSize(new_bounds));
}

void GaneshView::Draw(const Size& size) {
  GaneshTexture texture(gr_context_.context(), size);
  SkCanvas* canvas = texture.canvas();

  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setFlags(SkPaint::kAntiAlias_Flag);
  canvas->drawCircle(50, 100, 100, paint);
  canvas->flush();

  texture_uploader_.Upload(texture.texture_id(), size);
}

}  // namespace examples
}  // namespace mojo
