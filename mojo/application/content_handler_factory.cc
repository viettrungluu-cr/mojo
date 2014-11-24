// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/application/content_handler_factory.h"

#include <set>

#include "base/bind.h"
#include "base/callback.h"
#include "base/threading/platform_thread.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/common/message_pump_mojo.h"
#include "mojo/public/cpp/application/application_connection.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/interface_factory_impl.h"
#include "mojo/public/cpp/bindings/interface_impl.h"
#include "mojo/services/public/interfaces/content_handler/content_handler.mojom.h"

namespace mojo {

namespace {

class ApplicationThread : public base::PlatformThread::Delegate {
 public:
  ApplicationThread(
      scoped_refptr<base::MessageLoopProxy> handler_thread,
      const base::Callback<void(ApplicationThread*)>& termination_callback,
      ContentHandlerFactory::Delegate* handler_delegate,
      ShellPtr shell,
      URLResponsePtr response)
      : handler_thread_(handler_thread),
        termination_callback_(termination_callback),
        handler_delegate_(handler_delegate),
        shell_(shell.Pass()),
        response_(response.Pass()) {}

 private:
  void ThreadMain() override {
    handler_delegate_->RunApplication(shell_.Pass(), response_.Pass());
    handler_thread_->PostTask(FROM_HERE,
                              base::Bind(termination_callback_, this));
  }

  scoped_refptr<base::MessageLoopProxy> handler_thread_;
  base::Callback<void(ApplicationThread*)> termination_callback_;
  ContentHandlerFactory::Delegate* handler_delegate_;
  ShellPtr shell_;
  URLResponsePtr response_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationThread);
};

class ContentHandlerImpl : public InterfaceImpl<ContentHandler> {
 public:
  explicit ContentHandlerImpl(ContentHandlerFactory::Delegate* delegate)
      : in_destructor_(false), delegate_(delegate) {}
  ~ContentHandlerImpl() {
    in_destructor_ = true;
    for (auto thread : active_threads_) {
      base::PlatformThread::Join(thread.second);
      delete thread.first;
    }
  }

 private:
  // Overridden from ContentHandler:
  virtual void StartApplication(ShellPtr shell,
                                URLResponsePtr response) override {
    ApplicationThread* thread = new ApplicationThread(
        base::MessageLoopProxy::current(),
        base::Bind(&ContentHandlerImpl::OnThreadEnd, base::Unretained(this)),
        delegate_,
        shell.Pass(),
        response.Pass());
    base::PlatformThreadHandle handle;
    bool launched = base::PlatformThread::Create(0, thread, &handle);
    DCHECK(launched);
    active_threads_[thread] = handle;
  }

  void OnThreadEnd(ApplicationThread* thread) {
    if (in_destructor_) {
      // The destructor is already taking care of cleaning up this thread.
      return;
    }
    DCHECK(active_threads_.find(thread) != active_threads_.end());
    base::PlatformThreadHandle handle = active_threads_[thread];
    active_threads_.erase(thread);
    base::PlatformThread::Join(handle);
    delete thread;
    if (!active_threads_.size()) {
      ApplicationImpl::Terminate();
    }
  }

  bool in_destructor_;
  ContentHandlerFactory::Delegate* delegate_;
  std::map<ApplicationThread*, base::PlatformThreadHandle> active_threads_;

  DISALLOW_COPY_AND_ASSIGN(ContentHandlerImpl);
};

}  // namespace

ContentHandlerFactory::ContentHandlerFactory(Delegate* delegate)
    : delegate_(delegate) {
}

ContentHandlerFactory::~ContentHandlerFactory() {
}

void ContentHandlerFactory::ManagedDelegate::RunApplication(
    ShellPtr shell,
    URLResponsePtr response) {
  base::MessageLoop loop(common::MessagePumpMojo::Create());
  auto application = this->CreateApplication(shell.Pass(), response.Pass());
  loop.Run();
}

void ContentHandlerFactory::Create(ApplicationConnection* connection,
                                   InterfaceRequest<ContentHandler> request) {
  BindToRequest(new ContentHandlerImpl(delegate_), &request);
}

}  // namespace mojo
