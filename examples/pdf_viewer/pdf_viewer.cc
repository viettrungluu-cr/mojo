// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/string_tokenizer.h"
#include "examples/bitmap_uploader/bitmap_uploader.h"
#include "mojo/application/application_runner_chromium.h"
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
#include "mojo/services/public/interfaces/input_events/input_events.mojom.h"
#include "mojo/services/public/interfaces/input_events/input_key_codes.mojom.h"
#include "third_party/pdfium/fpdfsdk/include/fpdf_ext.h"
#include "third_party/pdfium/fpdfsdk/include/fpdfview.h"
#include "v8/include/v8.h"

#define BACKGROUND_COLOR 0xFF888888

namespace mojo {
namespace examples {

class PDFViewer;

class PDFView : public ViewManagerDelegate, public ViewObserver {
 public:
  static void Spawn(URLResponsePtr response,
                    ServiceProviderImpl* exported_services,
                    scoped_ptr<ServiceProvider> imported_services,
                    Shell* shell) {
    // PDFView deletes itself when its View is destroyed.
    new PDFView(
        response.Pass(), exported_services, imported_services.Pass(), shell);
  }

 private:
  PDFView(URLResponsePtr response,
          ServiceProviderImpl* exported_services,
          scoped_ptr<ServiceProvider> imported_services,
          Shell* shell)
      : current_page_(0),
        page_count_(0),
        doc_(NULL),
        imported_services_(imported_services.Pass()),
        shell_(shell),
        root_(nullptr),
        view_manager_client_factory_(shell, this) {
    exported_services->AddService(&view_manager_client_factory_);
    FetchPDF(response.Pass());
  }

  virtual ~PDFView() {
    if (doc_)
      FPDF_CloseDocument(doc_);
    if (root_)
      root_->RemoveObserver(this);
  }

  // Overridden from ViewManagerDelegate:
  virtual void OnEmbed(ViewManager* view_manager,
                       View* root,
                       ServiceProviderImpl* exported_services,
                       scoped_ptr<ServiceProvider> imported_services) override {
    root_ = root;
    root_->AddObserver(this);
    bitmap_uploader_.reset(new BitmapUploader(root_));
    bitmap_uploader_->Init(shell_);
    bitmap_uploader_->SetColor(BACKGROUND_COLOR);
    DrawBitmap();
  }

  virtual void OnViewManagerDisconnected(ViewManager* view_manager) override {
  }

  // Overridden from ViewObserver:
  virtual void OnViewBoundsChanged(View* view,
                                   const Rect& old_bounds,
                                   const Rect& new_bounds) override {
    DCHECK_EQ(view, root_);
    DrawBitmap();
  }

  virtual void OnViewInputEvent(View* view, const EventPtr& event) override {
    if (event->key_data &&
        (event->action != EVENT_TYPE_KEY_PRESSED ||
         event->key_data->is_char)) {
      return;
    }
    if ((event->key_data &&
         event->key_data->windows_key_code == KEYBOARD_CODE_DOWN) ||
        (event->wheel_data && event->wheel_data->y_offset < 0)) {
      if (current_page_ < (page_count_ - 1)) {
        current_page_++;
        DrawBitmap();
      }
    } else if ((event->key_data &&
                event->key_data->windows_key_code == KEYBOARD_CODE_UP) ||
               (event->wheel_data && event->wheel_data->y_offset > 0)) {
      if (current_page_ > 0) {
        current_page_--;
        DrawBitmap();
      }
    }
  }

  virtual void OnViewDestroyed(View* view) override {
    DCHECK_EQ(view, root_);
    delete this;
  }

  void DrawBitmap() {
    if (!root_ || !doc_)
      return;

    FPDF_PAGE page = FPDF_LoadPage(doc_, current_page_);
    int width = static_cast<int>(FPDF_GetPageWidth(page));
    int height = static_cast<int>(FPDF_GetPageHeight(page));

    scoped_ptr<std::vector<unsigned char>> bitmap;
    bitmap.reset(new std::vector<unsigned char>);
    bitmap->resize(width * height * 4);

    FPDF_BITMAP f_bitmap = FPDFBitmap_CreateEx(
        width, height, FPDFBitmap_BGRA, &(*bitmap)[0], width * 4);
    FPDFBitmap_FillRect(f_bitmap, 0, 0, width, height, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(f_bitmap, page, 0, 0, width, height, 0, 0);
    FPDFBitmap_Destroy(f_bitmap);

    FPDF_ClosePage(page);

    bitmap_uploader_->SetBitmap(width, height, bitmap.Pass(),
                                BitmapUploader::BGRA);
  }

  void FetchPDF(URLResponsePtr response) {
    int content_length = GetContentLength(response->headers);
    scoped_ptr<unsigned char[]> data;
    data_.reset(new unsigned char[content_length]);
    unsigned char* buf = data_.get();
    uint32_t bytes_remaining = content_length;
    uint32_t num_bytes = bytes_remaining;
    while (bytes_remaining > 0) {
      MojoResult result = ReadDataRaw(
          response->body.get(), buf, &num_bytes, MOJO_READ_DATA_FLAG_NONE);
      if (result == MOJO_RESULT_SHOULD_WAIT) {
        Wait(response->body.get(),
             MOJO_HANDLE_SIGNAL_READABLE,
             MOJO_DEADLINE_INDEFINITE);
      } else if (result == MOJO_RESULT_OK) {
        buf += num_bytes;
        num_bytes = bytes_remaining -= num_bytes;
      } else {
        break;
      }
    }

    doc_ = FPDF_LoadMemDocument(data_.get(), content_length, NULL);
    page_count_ = FPDF_GetPageCount(doc_);
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

  scoped_ptr<unsigned char[]> data_;
  int current_page_;
  int page_count_;
  FPDF_DOCUMENT doc_;
  scoped_ptr<ServiceProvider> imported_services_;
  Shell* shell_;
  View* root_;
  ViewManagerClientFactory view_manager_client_factory_;
  scoped_ptr<BitmapUploader> bitmap_uploader_;

  DISALLOW_COPY_AND_ASSIGN(PDFView);
};

class ContentHandlerImpl : public InterfaceImpl<ContentHandler> {
 public:
  explicit ContentHandlerImpl(Shell* shell) : shell_(shell) {}
  virtual ~ContentHandlerImpl() {}

 private:
  // Overridden from ContentHandler:
  virtual void OnConnect(
      const mojo::String& requestor_url,
      URLResponsePtr response,
      InterfaceRequest<ServiceProvider> service_provider) override {
    ServiceProviderImpl* exported_services = new ServiceProviderImpl();
    BindToRequest(exported_services, &service_provider);
    scoped_ptr<ServiceProvider> remote(
        exported_services->CreateRemoteServiceProvider());
    PDFView::Spawn(response.Pass(), exported_services, remote.Pass(), shell_);
  }

  Shell* shell_;

  DISALLOW_COPY_AND_ASSIGN(ContentHandlerImpl);
};

class PDFViewer : public ApplicationDelegate {
 public:
  PDFViewer() {}
  virtual ~PDFViewer() {
    FPDF_DestroyLibrary();
  }
 private:
  // Overridden from ApplicationDelegate:
  virtual void Initialize(ApplicationImpl* app) override {
    content_handler_factory_.reset(
        new InterfaceFactoryImplWithContext<ContentHandlerImpl, Shell>(
            app->shell()));

    v8::V8::InitializeICU();
    FPDF_InitLibrary(NULL);
  }

  // Overridden from ApplicationDelegate:
  virtual bool ConfigureIncomingConnection(
      ApplicationConnection* connection) override {
    connection->AddService(content_handler_factory_.get());
    return true;
  }

  scoped_ptr<InterfaceFactoryImplWithContext<ContentHandlerImpl, Shell> >
      content_handler_factory_;

  DISALLOW_COPY_AND_ASSIGN(PDFViewer);
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(new mojo::examples::PDFViewer);
  return runner.Run(shell_handle);
}
