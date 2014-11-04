// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "mojo/edk/embedder/embedder.h"
#include "mojo/edk/embedder/simple_platform_support.h"
#include "mojo/shell/external_application_registrar_connection.h"
#include "mojo/shell/in_process_dynamic_service_runner.h"
#include "url/gurl.h"

namespace {
const char kAppPath[] = "app-path";
const char kAppURL[] = "app-url";
const char kShellPath[] = "shell-path";
}

class Launcher {
 public:
  explicit Launcher(base::CommandLine* command_line)
      : app_path_(command_line->GetSwitchValueASCII(kAppPath)),
        app_url_(command_line->GetSwitchValueASCII(kAppURL)),
        loop_(base::MessageLoop::TYPE_IO),
        connection_(base::FilePath(
            command_line->GetSwitchValueASCII(kShellPath))),
        connect_result_(0) {}
  ~Launcher() {}

  int Connect() {
    DCHECK(!run_loop_.get());
    run_loop_.reset(new base::RunLoop);
    connection_.Connect(
        base::Bind(&Launcher::OnConnected, base::Unretained(this)));
    run_loop_->Run();
    run_loop_.reset();
    return connect_result_;
  }

  bool Register() {
    DCHECK(!run_loop_.get());
    DCHECK(connect_result_ == 0);
    run_loop_.reset(new base::RunLoop);
    connection_.Register(
        app_url_, base::Bind(&Launcher::OnRegistered, base::Unretained(this)));
    run_loop_->Run();
    run_loop_.reset();
    return shell_handle_.is_valid();
  }

  void Run() {
    DCHECK(!run_loop_.get());
    DCHECK(shell_handle_.is_valid());
    mojo::shell::InProcessDynamicServiceRunner service_runner(nullptr);
    run_loop_.reset(new base::RunLoop);
    service_runner.Start(
        app_path_,
        shell_handle_.Pass(),
        base::Bind(&Launcher::OnAppCompleted, base::Unretained(this)));
    run_loop_->Run();
    run_loop_.reset();
  }

 private:
  void OnConnected(int result) {
    connect_result_ = result;
    run_loop_->Quit();
  }

  void OnRegistered(mojo::ShellPtr shell) {
    shell_handle_ = shell.PassMessagePipe();
    run_loop_->Quit();
  }

  void OnAppCompleted() {
    run_loop_->Quit();
  }

  const base::FilePath app_path_;
  const GURL app_url_;
  base::MessageLoop loop_;
  mojo::shell::ExternalApplicationRegistrarConnection connection_;
  int connect_result_;
  mojo::ScopedMessagePipeHandle shell_handle_;
  scoped_ptr<base::RunLoop> run_loop_;
};

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  mojo::embedder::Init(scoped_ptr<mojo::embedder::PlatformSupport>(
      new mojo::embedder::SimplePlatformSupport()));
  base::CommandLine command_line(argc, argv);
  Launcher launcher(&command_line);
  int result = launcher.Connect();
  if (result < 0) {
    LOG(ERROR) << "Error(" << result << ") connecting on socket "
               << command_line.GetSwitchValueASCII(kShellPath);
    return MOJO_RESULT_INVALID_ARGUMENT;
  }

  if (!launcher.Register()) {
    LOG(ERROR) << "Error registering "
               << command_line.GetSwitchValueASCII(kAppURL);
    return MOJO_RESULT_INVALID_ARGUMENT;
  }

  launcher.Run();
  return MOJO_RESULT_OK;
}
