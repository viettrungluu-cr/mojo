// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GANESH_APP_TEXTURE_UPLOADER_H_
#define EXAMPLES_GANESH_APP_TEXTURE_UPLOADER_H_

#include "base/containers/hash_tables.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/c/gles2/gles2.h"
#include "mojo/services/public/interfaces/geometry/geometry.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surface_id.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surfaces.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surfaces_service.mojom.h"

namespace mojo {
class Shell;
}

namespace examples {

class TextureUploader : public mojo::SurfaceClient {
 public:
  class Client {
   public:
    virtual void OnSurfaceIdAvailable(mojo::SurfaceIdPtr surface_id) = 0;

   protected:
    virtual ~Client();
  };

  TextureUploader(Client* client,
                  mojo::Shell* shell,
                  MojoGLES2Context gles2_context);
  ~TextureUploader();

  void Upload(uint32_t texture_id, mojo::Size size);

 private:
  struct PendingUpload {
    PendingUpload(uint32_t tex, mojo::Size sz) : texture_id(tex), size(sz) {}

    uint32_t texture_id;
    mojo::Size size;
  };

  void ReturnResources(
      mojo::Array<mojo::ReturnedResourcePtr> resources) override;
  void OnSurfaceConnectionCreated(mojo::SurfacePtr surface,
                                  uint32_t id_namespace);
  void EnsureSurfaceForSize(const mojo::Size& size);

  Client* client_;
  MojoGLES2Context gles2_context_;
  mojo::SurfacesServicePtr surfaces_service_;
  scoped_ptr<PendingUpload> pending_upload_;
  mojo::SurfacePtr surface_;
  mojo::Size surface_size_;
  uint32_t next_resource_id_;
  uint32_t id_namespace_;
  mojo::SurfaceIdPtr surface_id_;
  base::hash_map<uint32_t, uint32_t> resource_to_texture_id_map_;

  base::WeakPtrFactory<TextureUploader> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(TextureUploader);
};

}  // namespace examples

#endif  // EXAMPLES_GANESH_APP_TEXTURE_UPLOADER_H_
