// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_tokenizer.h"
#include "examples/bitmap_uploader/bitmap_uploader.h"
#include "examples/media_viewer/media_viewer.mojom.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/application/content_handler_factory.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/public/cpp/application/service_provider_impl.h"
#include "mojo/services/public/cpp/view_manager/types.h"
#include "mojo/services/public/cpp/view_manager/view.h"
#include "mojo/services/public/cpp/view_manager/view_manager.h"
#include "mojo/services/public/cpp/view_manager/view_manager_client_factory.h"
#include "mojo/services/public/cpp/view_manager/view_manager_delegate.h"
#include "mojo/services/public/cpp/view_manager/view_observer.h"
#include "mojo/services/public/interfaces/content_handler/content_handler.mojom.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/codec/png_codec.h"

namespace mojo {
namespace examples {

namespace {

class EmbedderData {
 public:
  EmbedderData(Shell* shell, View* root) : bitmap_uploader_(root) {
    bitmap_uploader_.Init(shell);
    bitmap_uploader_.SetColor(SK_ColorGRAY);
  }

  BitmapUploader& bitmap_uploader() { return bitmap_uploader_; }

 private:
  BitmapUploader bitmap_uploader_;

  DISALLOW_COPY_AND_ASSIGN(EmbedderData);
};

}  // namespace

// TODO(aa): Hook up ZoomableMedia interface again.
class PNGView : public ApplicationDelegate,
                public ViewManagerDelegate,
                public ViewObserver {
 public:
  PNGView(URLResponsePtr response)
      : width_(0),
        height_(0),
        app_(nullptr),
        zoom_percentage_(kDefaultZoomPercentage) {
    DecodePNG(response.Pass());
  }

  virtual ~PNGView() {
    for (auto& roots : embedder_for_roots_) {
      roots.first->RemoveObserver(this);
      delete roots.second;
    }
  }

 private:
  static const uint16_t kMaxZoomPercentage = 400;
  static const uint16_t kMinZoomPercentage = 20;
  static const uint16_t kDefaultZoomPercentage = 100;
  static const uint16_t kZoomStep = 20;

  // Overridden from ApplicationDelegate:
  virtual void Initialize(ApplicationImpl* app) override {
    app_ = app;
    view_manager_client_factory_.reset(
        new ViewManagerClientFactory(app->shell(), this));
  }

  // Overridden from ApplicationDelegate:
  virtual bool ConfigureIncomingConnection(
      ApplicationConnection* connection) override {
    connection->AddService(view_manager_client_factory_.get());
    return true;
  }

  // Overridden from ViewManagerDelegate:
  virtual void OnEmbed(ViewManager* view_manager,
                       View* root,
                       ServiceProviderImpl* exported_services,
                       scoped_ptr<ServiceProvider> imported_services) override {
    // TODO(qsr): The same view should be embeddable on multiple views.
    DCHECK(embedder_for_roots_.find(root) == embedder_for_roots_.end());
    root->AddObserver(this);
    EmbedderData* embedder_data = new EmbedderData(app_->shell(), root);
    embedder_for_roots_[root] = embedder_data;
    embedder_data->bitmap_uploader().SetBitmap(
        width_, height_,
        make_scoped_ptr(new std::vector<unsigned char>(*bitmap_)),
        BitmapUploader::BGRA);
  }

  virtual void OnViewManagerDisconnected(ViewManager* view_manager) override {
  }

  // Overridden from ViewObserver:
  virtual void OnViewBoundsChanged(View* view,
                                   const Rect& old_bounds,
                                   const Rect& new_bounds) override {
    DCHECK(embedder_for_roots_.find(view) != embedder_for_roots_.end());
  }

  virtual void OnViewDestroyed(View* view) override {
    // TODO(aa): Need to figure out how shutdown works.
    const auto& it = embedder_for_roots_.find(view);
    DCHECK(it != embedder_for_roots_.end());
    delete it->second;
    embedder_for_roots_.erase(it);
    if (embedder_for_roots_.size() == 0)
      ApplicationImpl::Terminate();
  }

  void ZoomIn() {
    // TODO(qsr,aa) Zoom should be per embedder view.
    if (zoom_percentage_ >= kMaxZoomPercentage)
      return;
    zoom_percentage_ += kZoomStep;
  }

  void ZoomOut() {
    if (zoom_percentage_ <= kMinZoomPercentage)
      return;
    zoom_percentage_ -= kZoomStep;
  }

  void ZoomToActualSize() {
    if (zoom_percentage_ == kDefaultZoomPercentage)
      return;
    zoom_percentage_ = kDefaultZoomPercentage;
  }

  void DecodePNG(URLResponsePtr response) {
    int content_length = GetContentLength(response->headers);
    scoped_ptr<unsigned char[]> data(new unsigned char[content_length]);
    unsigned char* buf = data.get();
    uint32_t bytes_remaining = content_length;
    uint32_t num_bytes = bytes_remaining;
    while (bytes_remaining > 0) {
      MojoResult result = ReadDataRaw(response->body.get(), buf, &num_bytes,
                                      MOJO_READ_DATA_FLAG_NONE);
      if (result == MOJO_RESULT_SHOULD_WAIT) {
        Wait(response->body.get(), MOJO_HANDLE_SIGNAL_READABLE,
             MOJO_DEADLINE_INDEFINITE);
      } else if (result == MOJO_RESULT_OK) {
        buf += num_bytes;
        num_bytes = bytes_remaining -= num_bytes;
      } else {
        break;
      }
    }

    bitmap_.reset(new std::vector<unsigned char>);
    gfx::PNGCodec::Decode(static_cast<const unsigned char*>(data.get()),
                          content_length, gfx::PNGCodec::FORMAT_BGRA,
                          bitmap_.get(), &width_, &height_);
  }

  int GetContentLength(const Array<String>& headers) {
    for (size_t i = 0; i < headers.size(); ++i) {
      base::StringTokenizer t(headers[i], ": ;=");
      while (t.GetNext()) {
        if (!t.token_is_delim() && t.token() == "Content-Length") {
          while (t.GetNext()) {
            if (!t.token_is_delim())
              return atoi(t.token().c_str());
          }
        }
      }
    }
    return 0;
  }

  int width_;
  int height_;
  scoped_ptr<std::vector<unsigned char>> bitmap_;
  ApplicationImpl* app_;
  std::map<View*, EmbedderData*> embedder_for_roots_;
  scoped_ptr<ViewManagerClientFactory> view_manager_client_factory_;
  uint16_t zoom_percentage_;

  DISALLOW_COPY_AND_ASSIGN(PNGView);
};

class PNGViewer : public ApplicationDelegate,
                  public ContentHandlerFactory::Delegate {
 public:
  PNGViewer() : content_handler_factory_(this) {}

 private:
  // Overridden from ApplicationDelegate:
  virtual bool ConfigureIncomingConnection(
      ApplicationConnection* connection) override {
    connection->AddService(&content_handler_factory_);
    return true;
  }

  // Overridden from ContentHandlerFactory::Delegate:
  virtual scoped_ptr<ContentHandlerFactory::HandledApplicationHolder>
  CreateApplication(ShellPtr shell, URLResponsePtr response) override {
    return make_handled_factory_holder(new mojo::ApplicationImpl(
        new PNGView(response.Pass()), shell.PassMessagePipe()));
  }

  ContentHandlerFactory content_handler_factory_;

  DISALLOW_COPY_AND_ASSIGN(PNGViewer);
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::examples::PNGViewer());
  return runner.Run(shell_handle);
}
