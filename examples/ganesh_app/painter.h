// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GANESH_APP_PAINTER_H_
#define EXAMPLES_GANESH_APP_PAINTER_H_

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "cc/surfaces/surface_id.h"
#include "cc/surfaces/surface_id_allocator.h"
#include "examples/surfaces_app/child.mojom.h"
#include "mojo/public/c/gles2/gles2.h"
#include "mojo/services/public/interfaces/geometry/geometry.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surface_id.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surfaces.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surfaces_service.mojom.h"
#include "skia/ext/refptr.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "ui/gfx/size.h"

namespace mojo {
class ApplicationConnection;

namespace examples {

class Painter : public InterfaceImpl<Child>, public SurfaceClient {
 public:
  Painter(ApplicationConnection* surfaces_service_connection,
          CommandBufferPtr command_buffer);
  ~Painter();

  // SurfaceClient implementation
  virtual void ReturnResources(Array<ReturnedResourcePtr> resources) override;

 private:
  // Child implementation.
  virtual void ProduceFrame(
      ColorPtr color,
      SizePtr size,
      const mojo::Callback<void(SurfaceIdPtr id)>& callback) override;

  void SurfaceConnectionCreated(SurfacePtr surface, uint32_t id_namespace);
  void AllocateSurface();
  void Draw();

  skia::RefPtr<GrContext> gr_context_;

  gfx::Size size_;
  scoped_ptr<cc::SurfaceIdAllocator> allocator_;
  SurfacesServicePtr surfaces_service_;
  SurfacePtr surface_;
  MojoGLES2Context context_;
  cc::SurfaceId id_;
  Callback<void(SurfaceIdPtr id)> produce_callback_;
  uint32_t next_resource_id_;
  base::hash_map<uint32_t, GLuint> id_to_tex_map_;

  base::WeakPtrFactory<Painter> weak_factory_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(Painter);
};

}  // namespace examples
}  // namespace mojo

#endif  // EXAMPLES_GANESH_APP_PAINTER_H_
