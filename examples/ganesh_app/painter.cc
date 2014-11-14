// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/ganesh_app/painter.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "cc/output/compositor_frame.h"
#include "cc/output/delegated_frame_data.h"
#include "cc/quads/render_pass.h"
#include "cc/quads/texture_draw_quad.h"
#include "examples/surfaces_app/surfaces_util.h"
#include "gpu/GLES2/gl2chromium.h"
#include "gpu/GLES2/gl2extchromium.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/client/gles2_lib.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/mailbox_holder.h"
#include "gpu/skia_bindings/gl_bindings_skia_cmd_buffer.h"
#include "mojo/converters/geometry/geometry_type_converters.h"
#include "mojo/converters/surfaces/surfaces_type_converters.h"
#include "mojo/public/c/gles2/gles2.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/environment/environment.h"
#include "mojo/services/public/interfaces/surfaces/surface_id.mojom.h"
#include "mojo/services/public/interfaces/surfaces/surfaces.mojom.h"
#include "third_party/khronos/GLES2/gl2.h"
#include "third_party/khronos/GLES2/gl2ext.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/transform.h"

using cc::RenderPass;
using cc::RenderPassId;
using cc::DrawQuad;
using cc::TextureDrawQuad;
using cc::DelegatedFrameData;
using cc::CompositorFrame;

namespace mojo {
namespace examples {
namespace {

// The limit of the number of GPU resources we hold in the GrContext's
// GPU cache.
const int kMaxGaneshResourceCacheCount = 2048;

// The limit of the bytes allocated toward GPU resources in the GrContext's
// GPU cache.
const size_t kMaxGaneshResourceCacheBytes = 96 * 1024 * 1024;

}

static void ContextLostThunk(void*) {
  LOG(FATAL) << "Context lost";
}

Painter::Painter(ApplicationConnection* surfaces_service_connection,
                 CommandBufferPtr command_buffer)
    : next_resource_id_(0),
      weak_factory_(this) {
  surfaces_service_connection->ConnectToService(&surfaces_service_);
  surfaces_service_->CreateSurfaceConnection(base::Bind(
      &Painter::SurfaceConnectionCreated, weak_factory_.GetWeakPtr()));
  context_ =
      MojoGLES2CreateContext(command_buffer.PassMessagePipe().release().value(),
                             &ContextLostThunk,
                             this,
                             mojo::Environment::GetDefaultAsyncWaiter());
  DCHECK(context_);
  MojoGLES2MakeCurrent(context_);

  gpu::gles2::GLES2Interface* gl = static_cast<gpu::gles2::GLES2Interface*>(
      MojoGLES2GetGLES2Interface(context_));
  gles2::SetGLContext(gl);

  skia::RefPtr<GrGLInterface> interface = skia::AdoptRef(
      skia_bindings::CreateCommandBufferSkiaGLBinding());
  DCHECK(interface);

  gr_context_ = skia::AdoptRef(GrContext::Create(
      kOpenGL_GrBackend,
      reinterpret_cast<GrBackendContext>(interface.get())));
  DCHECK(gr_context_);
  gr_context_->setResourceCacheLimits(kMaxGaneshResourceCacheCount,
                                      kMaxGaneshResourceCacheBytes);
}

Painter::~Painter() {
  MojoGLES2DestroyContext(context_);
  surface_->DestroySurface(mojo::SurfaceId::From(id_));
}

void Painter::ProduceFrame(
    ColorPtr color,
    SizePtr size,
    const mojo::Callback<void(SurfaceIdPtr id)>& callback) {

  size_ = size.To<gfx::Size>();
  produce_callback_ = callback;
  AllocateSurface();
}

void Painter::SurfaceConnectionCreated(SurfacePtr surface,
                                       uint32_t id_namespace) {
  surface_ = surface.Pass();
  surface_.set_client(this);
  allocator_.reset(new cc::SurfaceIdAllocator(id_namespace));
  AllocateSurface();
}

void Painter::ReturnResources(Array<ReturnedResourcePtr> resources) {
  for (size_t i = 0; i < resources.size(); ++i) {
    cc::ReturnedResource res = resources[i].To<cc::ReturnedResource>();
    GLuint returned_texture = id_to_tex_map_[res.id];
    glDeleteTextures(1, &returned_texture);
  }
}

void Painter::AllocateSurface() {
  if (produce_callback_.is_null() || !allocator_)
    return;

  id_ = allocator_->GenerateId();
  surface_->CreateSurface(mojo::SurfaceId::From(id_), mojo::Size::From(size_));
  produce_callback_.Run(SurfaceId::From(id_));
  Draw();
}

void Painter::Draw() {
  gpu::gles2::GLES2Interface* gl = static_cast<gpu::gles2::GLES2Interface*>(
      MojoGLES2GetGLES2Interface(context_));

  GLuint texture_id = 0u;
  gl->GenTextures(1, &texture_id);
  gl->BindTexture(GL_TEXTURE_2D, texture_id);
  gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_.width(), size_.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  GrBackendTextureDesc desc;
  desc.fFlags = kRenderTarget_GrBackendTextureFlag;
  desc.fWidth = size_.width();
  desc.fHeight = size_.height();
  desc.fConfig = kSkia8888_GrPixelConfig;
  desc.fOrigin = kTopLeft_GrSurfaceOrigin;
  desc.fTextureHandle = texture_id;
  skia::RefPtr<GrTexture> texture = skia::AdoptRef(
      gr_context_->wrapBackendTexture(desc));

  DCHECK(texture) << "No texture";
  DCHECK(texture->asRenderTarget()) << "No render target";

  skia::RefPtr<SkSurface> sk_surface = skia::AdoptRef(
      SkSurface::NewRenderTargetDirect(texture->asRenderTarget()));

  DCHECK(sk_surface);

  SkCanvas* canvas = sk_surface->getCanvas();

  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setFlags(SkPaint::kAntiAlias_Flag);
  canvas->drawCircle(size_.width() / 2, 0, 100, paint);
  canvas->flush();

  // Then, put the texture into a mailbox.
  gpu::Mailbox mailbox = gpu::Mailbox::Generate();
  glProduceTextureCHROMIUM(GL_TEXTURE_2D, mailbox.name);
  GLuint sync_point = glInsertSyncPointCHROMIUM();
  gpu::MailboxHolder holder(mailbox, GL_TEXTURE_2D, sync_point);

  // Then, put the mailbox into a TransferableResource
  cc::TransferableResource resource;
  resource.id = next_resource_id_++;
  id_to_tex_map_[resource.id] = texture_id;
  resource.format = cc::RGBA_8888;
  resource.filter = GL_LINEAR;
  resource.size = size_;
  resource.mailbox_holder = holder;
  resource.is_repeated = false;
  resource.is_software = false;

  gfx::Rect rect(size_);
  cc::RenderPassId id(1, 1);
  scoped_ptr<RenderPass> pass = RenderPass::Create();
  pass->SetNew(id, rect, rect, gfx::Transform());

  CreateAndAppendSimpleSharedQuadState(pass.get(), gfx::Transform(), size_);

  TextureDrawQuad* texture_quad =
      pass->CreateAndAppendDrawQuad<TextureDrawQuad>();
  float vertex_opacity[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  texture_quad->SetNew(pass->shared_quad_state_list.back(),
                       rect,
                       rect,
                       rect,
                       resource.id,
                       true,
                       gfx::PointF(),
                       gfx::PointF(1.f, 1.f),
                       SK_ColorBLUE,
                       vertex_opacity,
                       false);

  scoped_ptr<DelegatedFrameData> delegated_frame_data(new DelegatedFrameData);
  delegated_frame_data->render_pass_list.push_back(pass.Pass());
  delegated_frame_data->resource_list.push_back(resource);

  scoped_ptr<CompositorFrame> frame(new CompositorFrame);
  frame->delegated_frame_data = delegated_frame_data.Pass();

  surface_->SubmitFrame(mojo::SurfaceId::From(id_), mojo::Frame::From(*frame));

  base::MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&Painter::Draw, weak_factory_.GetWeakPtr()),
      base::TimeDelta::FromMilliseconds(50));
}

}  // namespace examples
}  // namespace mojo
