// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/shell/domain_socket/unix_domain_server_socket_posix.h"

#include <vector>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/stl_util.h"
#include "mojo/shell/domain_socket/net_errors.h"
#include "mojo/shell/domain_socket/test_completion_callback.h"
#include "mojo/shell/domain_socket/unix_domain_client_socket_posix.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace mojo {
namespace shell {
namespace {

const char kSocketFilename[] = "socket_for_testing";
const char kInvalidSocketPath[] = "/invalid/path";

bool UserCanConnectCallback(
    bool allow_user,
    const UnixDomainServerSocket::Credentials& credentials) {
  // Here peers are running in same process.
  EXPECT_EQ(getpid(), credentials.process_id);
  EXPECT_EQ(getuid(), credentials.user_id);
  EXPECT_EQ(getgid(), credentials.group_id);
  return allow_user;
}

UnixDomainServerSocket::AuthCallback CreateAuthCallback(bool allow_user) {
  return base::Bind(&UserCanConnectCallback, allow_user);
}
}  // namespace

class UnixDomainServerSocketTest : public testing::Test {
 protected:
  UnixDomainServerSocketTest() {
    EXPECT_TRUE(temp_dir_.CreateUniqueTempDir());
    socket_path_ = temp_dir_.path().Append(kSocketFilename).value();
  }

  base::ScopedTempDir temp_dir_;
  std::string socket_path_;
  base::MessageLoopForIO loop_;
};

TEST_F(UnixDomainServerSocketTest, ListenWithInvalidPath) {
  const bool kUseAbstractNamespace = false;
  UnixDomainServerSocket server_socket(CreateAuthCallback(true),
                                       kUseAbstractNamespace);
  EXPECT_EQ(net::ERR_FILE_NOT_FOUND,
            server_socket.ListenWithPath(kInvalidSocketPath, 1));
}

TEST_F(UnixDomainServerSocketTest, ListenWithInvalidPathWithAbstractNamespace) {
  const bool kUseAbstractNamespace = true;
  UnixDomainServerSocket server_socket(CreateAuthCallback(true),
                                       kUseAbstractNamespace);
  EXPECT_EQ(net::OK, server_socket.ListenWithPath(kInvalidSocketPath, 1));
}

TEST_F(UnixDomainServerSocketTest, ListenAgainAfterFailureWithInvalidPath) {
  const bool kUseAbstractNamespace = false;
  UnixDomainServerSocket server_socket(CreateAuthCallback(true),
                                       kUseAbstractNamespace);
  EXPECT_EQ(net::ERR_FILE_NOT_FOUND,
            server_socket.ListenWithPath(kInvalidSocketPath, 1));
  EXPECT_EQ(net::OK, server_socket.ListenWithPath(socket_path_, 1));
}

TEST_F(UnixDomainServerSocketTest, AcceptWithForbiddenUser) {
  const bool kUseAbstractNamespace = false;

  UnixDomainServerSocket server_socket(CreateAuthCallback(false),
                                       kUseAbstractNamespace);
  EXPECT_EQ(net::OK, server_socket.ListenWithPath(socket_path_, 1));

  SocketDescriptor accepted_socket = kInvalidSocket;
  TestCompletionCallback accept_callback;
  EXPECT_EQ(net::ERR_IO_PENDING,
            server_socket.Accept(&accepted_socket, accept_callback.callback()));
  EXPECT_EQ(accepted_socket, kInvalidSocket);

  UnixDomainClientSocket client_socket(socket_path_, kUseAbstractNamespace);
  EXPECT_FALSE(client_socket.IsConnected());

  // Connect() will return net::OK before the server rejects the connection.
  TestCompletionCallback connect_callback;
  int rv = connect_callback.GetResult(
      client_socket.Connect(connect_callback.callback()));
  ASSERT_EQ(net::OK, rv);

  // Run message loop so server can process incoming connection attempt.
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_FALSE(client_socket.IsConnected());

  // The server socket should not have called |accept_callback| or modified
  // |accepted_socket|.
  EXPECT_FALSE(accept_callback.have_result());
  EXPECT_EQ(accepted_socket, kInvalidSocket);
}

// Normal cases including read/write are tested by UnixDomainClientSocketTest.

}  // namespace shell
}  // namespace mojo
