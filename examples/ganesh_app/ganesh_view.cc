// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/ganesh_view.h"

#include "mojo/skia/ganesh_surface.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace examples {
namespace {

mojo::Size ToSize(const mojo::Rect& rect) {
  mojo::Size size;
  size.width = rect.width;
  size.height = rect.height;
  return size;
}
}

GaneshView::GaneshView(mojo::Shell* shell, mojo::View* view)
    : view_(view),
      gl_context_(mojo::GLContext::Create(shell)),
      gr_context_(gl_context_),
      texture_uploader_(this, shell, gl_context_) {
  Draw(ToSize(view_->bounds()));
}

GaneshView::~GaneshView() {
  if (gl_context_)
    gl_context_->Destroy();
}

void GaneshView::OnSurfaceIdAvailable(mojo::SurfaceIdPtr surface_id) {
  view_->SetSurfaceId(surface_id.Pass());
}

void GaneshView::OnViewDestroyed(mojo::View* view) {
  delete this;
}

void GaneshView::OnViewBoundsChanged(mojo::View* view,
                                     const mojo::Rect& old_bounds,
                                     const mojo::Rect& new_bounds) {
  Draw(ToSize(new_bounds));
}

void GaneshView::Draw(const mojo::Size& size) {
  mojo::GaneshContext::Scope scope(&gr_context_);
  mojo::GaneshSurface surface(
      &gr_context_,
      make_scoped_ptr(new mojo::GLTexture(gl_context_, size)));

  SkCanvas* canvas = surface.canvas();

  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setFlags(SkPaint::kAntiAlias_Flag);
  canvas->drawCircle(50, 100, 100, paint);
  canvas->flush();

  texture_uploader_.Upload(surface.TakeTexture());
}

}  // namespace examples
