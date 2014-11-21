// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdlib>

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/time/time.h"
#include "examples/indirect_service/indirect_service_demo.mojom.h"
#include "mojo/application/application_runner_chromium.h"
#include "mojo/common/message_pump_mojo.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"

namespace mojo {
namespace examples {

class DemoTask;

typedef typename base::Callback<void(DemoTask*, const std::vector<int32_t>&)>
    DemoTaskFinishedCallback;

// A thread that connects to the IndirectIntegerService, gets a connection
// to its IntegerService, and then calls Increment() iteration_count times.
// The results are saved and returned with the finished_callback.
class DemoTask {
 public:
  DemoTask(ScopedMessagePipeHandle proxy_handle,
           const DemoTaskFinishedCallback& finished_callback,
           unsigned iteration_count)
    : proxy_handle_(proxy_handle.Pass()),
      thread_("DemoTask"),
      finished_callback_(finished_callback),
      iteration_count_(iteration_count) {

    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_CUSTOM;
    options.message_pump_factory = base::Bind(&common::MessagePumpMojo::Create);
    CHECK(thread_.StartWithOptions(options));

    thread_.message_loop()->PostTask(
        FROM_HERE, base::Bind(&DemoTask::Run, base::Unretained(this)));
  }

  void Run() {
    integer_service_.Bind(proxy_handle_.Pass());
    base::Callback<void(int32_t)> callback =
        base::Bind(&DemoTask::SaveResultAndFinish, base::Unretained(this));
    for(int unsigned i = 0; i < iteration_count_; i++) {
      integer_service_->Increment(callback);
      // To ensure that the DemoTask threads' execution overlaps, sleep.
      if (i < iteration_count_ - 1)
        base::PlatformThread::Sleep(
            base::TimeDelta::FromMilliseconds(rand() % 10));
    }
  }

 private:
  void SaveResultAndFinish(int32_t result) {
    results_.push_back(result);
    if (results_.size() == iteration_count_) {
      integer_service_.reset(); // Must be done on thread_.
      finished_callback_.Run(this, results_);
    }
  }

  ScopedMessagePipeHandle proxy_handle_;
  base::Thread thread_;
  IntegerServicePtr integer_service_;
  DemoTaskFinishedCallback finished_callback_;
  unsigned iteration_count_;
  std::vector<int32_t> results_;
};

// Connect to the IntegerService and give its proxy to the
// IndirectIntegerService. Start kTaskCount DemoTask threads all of
// which will use the IndirectIntegerService to get their own connection
// to the (one) IntegerService. Each DemoTask will call the IntegerService's
// Increment() method kTaskIterationCount times, collect the results in
// a vector and return them to FinishDemoTask.
//
// The IntegerService, whose value is initially 0, will be called a total of
// N = |kTaskCount * kTaskIterationCount| times. Each DemoTask's results
// are displayed in array of length N. Digits appear in positions that
// correspond to the results obtained by the DemoTask thread. The results
// show that the DemoTask threads are accessing the Integer in parallel.
// The fact that only one digit appears in each column shows that things
// are working correctly.
class IndirectServiceDemoAppDelegate : public ApplicationDelegate {
 public:
  void Initialize(ApplicationImpl* app) override {
    IntegerServicePtr indirect_service_delegate;
    app->ConnectToService("mojo:indirect_integer_service",
        &indirect_integer_service_);
    app->ConnectToService("mojo:integer_service", &indirect_service_delegate);
    indirect_integer_service_->Set(indirect_service_delegate.Pass());

    for (unsigned i = 0; i < kTaskCount; i++) {
      IntegerServicePtr integer_service;
      indirect_integer_service_->Get(GetProxy(&integer_service));
      DemoTaskFinishedCallback finished_callback = base::Bind(
          &IndirectServiceDemoAppDelegate::FinishDemoTask,
          base::Unretained(this),
          base::Unretained(base::MessageLoop::current()));
      // We're passing the integer_service_ proxy to another thread, so
      // use its MessagePipe.
      tasks_.push_back(new DemoTask(integer_service.PassMessagePipe(),
                                    finished_callback,
                                    kTaskIterationCount));
    }
  }

 private:
  static const unsigned kTaskCount = 10;
  static const unsigned kTaskIterationCount = 6;

  // This method is called on a DemoTask thread. It just calls DoFinishDemoTask
  // on the application's run loop. Doing so serializes the DoFinishDemoTask
  // calls.
  void FinishDemoTask(base::MessageLoop *run_loop,
                      DemoTask* task,
                      const std::vector<int32_t>& results) {
    run_loop->PostTask(FROM_HERE, base::Bind(
        &IndirectServiceDemoAppDelegate::DoFinishDemoTask,
        base::Unretained(this),
        base::Unretained(task),
        results));
  }

  void DoFinishDemoTask(DemoTask* task, const std::vector<int32_t>& results) {
    std::string display(kTaskCount * kTaskIterationCount, ' ');
    for (unsigned i = 0; i < results.size(); i++)
      display[results[i]] = '0' + (results[i] % 10);
    printf("DemoTask Thread [%s]\n", display.c_str());
    tasks_.erase(std::remove(tasks_.begin(), tasks_.end(), task), tasks_.end());
    delete task; // Stop the DemoTask's thread etc.
    if (tasks_.empty())
      ApplicationImpl::Terminate();
  }

  IndirectIntegerServicePtr indirect_integer_service_;
  std::vector<DemoTask*> tasks_;
};


}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunnerChromium runner(
      new mojo::examples::IndirectServiceDemoAppDelegate);
  return runner.Run(shell_handle);
}
