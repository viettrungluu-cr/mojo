// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/texture_uploader.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "base/bind.h"
#include "gpu/GLES2/gl2chromium.h"
#include "gpu/GLES2/gl2extchromium.h"
#include "mojo/public/c/gles2/gles2.h"
#include "mojo/public/cpp/application/connect.h"
#include "mojo/public/interfaces/application/shell.mojom.h"
#include "mojo/services/public/cpp/geometry/geometry_util.h"
#include "mojo/services/public/cpp/surfaces/surfaces_utils.h"
#include "mojo/services/public/cpp/surfaces/surfaces_utils.h"

namespace mojo {
namespace examples {

TextureUploader::Client::~Client() {
}

TextureUploader::TextureUploader(Client* client,
                                 Shell* shell,
                                 MojoGLES2Context gles2_context)
    : client_(client),
      gles2_context_(gles2_context),
      next_resource_id_(0),
      id_namespace_(0),
      weak_factory_(this) {
  ServiceProviderPtr surfaces_service_provider;
  shell->ConnectToApplication("mojo:surfaces_service",
                              GetProxy(&surfaces_service_provider));
  ConnectToService(surfaces_service_provider.get(), &surfaces_service_);

  surfaces_service_->CreateSurfaceConnection(
      base::Bind(&TextureUploader::OnSurfaceConnectionCreated,
                 weak_factory_.GetWeakPtr()));
}

TextureUploader::~TextureUploader() {
  if (surface_id_)
    surface_->DestroySurface(surface_id_.Clone());
}

void TextureUploader::Upload(uint32_t texture_id, Size size) {
  if (!surface_) {
    pending_upload_.reset(new PendingUpload(texture_id, size));
    return;
  }

  EnsureSurfaceForSize(size);

  FramePtr frame = Frame::New();
  frame->resources.resize(0u);

  Rect bounds;
  bounds.width = size.width;
  bounds.height = size.height;
  PassPtr pass = CreateDefaultPass(1, bounds);
  pass->quads.resize(0u);
  pass->shared_quad_states.push_back(CreateDefaultSQS(size));

  MojoGLES2MakeCurrent(gles2_context_);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  GLbyte mailbox[GL_MAILBOX_SIZE_CHROMIUM];
  glGenMailboxCHROMIUM(mailbox);
  glProduceTextureCHROMIUM(GL_TEXTURE_2D, mailbox);
  GLuint sync_point = glInsertSyncPointCHROMIUM();

  TransferableResourcePtr resource = TransferableResource::New();
  resource->id = next_resource_id_++;
  resource_to_texture_id_map_[resource->id] = texture_id;
  resource->format = mojo::RESOURCE_FORMAT_RGBA_8888;
  resource->filter = GL_LINEAR;
  resource->size = size.Clone();
  MailboxHolderPtr mailbox_holder = MailboxHolder::New();
  mailbox_holder->mailbox = Mailbox::New();
  for (int i = 0; i < GL_MAILBOX_SIZE_CHROMIUM; ++i)
    mailbox_holder->mailbox->name.push_back(mailbox[i]);
  mailbox_holder->texture_target = GL_TEXTURE_2D;
  mailbox_holder->sync_point = sync_point;
  resource->mailbox_holder = mailbox_holder.Pass();
  resource->is_repeated = false;
  resource->is_software = false;

  QuadPtr quad = Quad::New();
  quad->material = MATERIAL_TEXTURE_CONTENT;

  RectPtr rect = Rect::New();
  rect->width = size.width;
  rect->height = size.height;
  quad->rect = rect.Clone();
  quad->opaque_rect = rect.Clone();
  quad->visible_rect = rect.Clone();
  quad->needs_blending = true;
  quad->shared_quad_state_index = 0u;

  TextureQuadStatePtr texture_state = TextureQuadState::New();
  texture_state->resource_id = resource->id;
  texture_state->premultiplied_alpha = true;
  texture_state->uv_top_left = PointF::New();
  texture_state->uv_bottom_right = PointF::New();
  texture_state->uv_bottom_right->x = 1.f;
  texture_state->uv_bottom_right->y = 1.f;
  texture_state->background_color = Color::New();
  texture_state->background_color->rgba = 0;
  for (int i = 0; i < 4; ++i)
    texture_state->vertex_opacity.push_back(1.f);
  texture_state->flipped = false;

  frame->resources.push_back(resource.Pass());
  quad->texture_quad_state = texture_state.Pass();
  pass->quads.push_back(quad.Pass());

  frame->passes.push_back(pass.Pass());
  surface_->SubmitFrame(surface_id_.Clone(), frame.Pass());
}

void TextureUploader::EnsureSurfaceForSize(const Size& size) {
  if (surface_id_ && size == surface_size_)
    return;

  if (surface_id_) {
    surface_->DestroySurface(surface_id_.Clone());
  } else {
    surface_id_ = SurfaceId::New();
    surface_id_->id = static_cast<uint64_t>(id_namespace_) << 32;
  }

  surface_id_->id++;
  surface_->CreateSurface(surface_id_.Clone(), size.Clone());
  client_->OnSurfaceIdAvailable(surface_id_.Clone());
  surface_size_ = size;
}

void TextureUploader::ReturnResources(Array<ReturnedResourcePtr> resources) {
  if (!resources.size())
    return;
  MojoGLES2MakeCurrent(gles2_context_);
  for (size_t i = 0u; i < resources.size(); ++i) {
    ReturnedResourcePtr resource = resources[i].Pass();
    DCHECK_EQ(1, resource->count);
    glWaitSyncPointCHROMIUM(resource->sync_point);
    uint32_t texture_id = resource_to_texture_id_map_[resource->id];
    DCHECK_NE(0u, texture_id);
    resource_to_texture_id_map_.erase(resource->id);
    glDeleteTextures(1, &texture_id);
  }
}

void TextureUploader::OnSurfaceConnectionCreated(SurfacePtr surface,
                                                 uint32_t id_namespace) {
  surface_ = surface.Pass();
  surface_.set_client(this);
  id_namespace_ = id_namespace;

  if (pending_upload_) {
    scoped_ptr<PendingUpload> pending = pending_upload_.Pass();
    Upload(pending->texture_id, pending->size);
  }
}

}  // namespace examples
}  // namespace mojo
