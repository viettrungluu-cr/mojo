// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GANESH_APP_GANESH_VIEW_H_
#define EXAMPLES_GANESH_APP_GANESH_VIEW_H_

#include "examples/ganesh_app/ganesh_context.h"
#include "examples/ganesh_app/mojo_gl_context.h"
#include "examples/ganesh_app/texture_uploader.h"
#include "mojo/services/public/cpp/view_manager/view_observer.h"
#include "mojo/services/public/interfaces/surfaces/surface_id.mojom.h"

namespace mojo {
class Shell;

namespace examples {

class GaneshView : public TextureUploader::Client, public ViewObserver {
 public:
  GaneshView(Shell* shell, View* view);
  ~GaneshView();

 private:
  void OnViewDestroyed(View* view) override;
  void OnViewBoundsChanged(View* view,
                           const Rect& old_bounds,
                           const Rect& new_bounds) override;

  void OnSurfaceIdAvailable(SurfaceIdPtr surface_id) override;

  void Draw(const Size& size);

  View* view_;
  MojoGLContext gl_context_;
  GaneshContext gr_context_;
  TextureUploader texture_uploader_;

  DISALLOW_COPY_AND_ASSIGN(GaneshView);
};

}  // namespace examples
}  // namespace mojo

#endif  // EXAMPLES_GANESH_APP_GANESH_VIEW_H_
