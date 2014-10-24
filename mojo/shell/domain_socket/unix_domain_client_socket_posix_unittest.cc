// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/shell/domain_socket/unix_domain_client_socket_posix.h"

#include <unistd.h>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/posix/eintr_wrapper.h"
#include "mojo/shell/domain_socket/net_errors.h"
#include "mojo/shell/domain_socket/socket_libevent.h"
#include "mojo/shell/domain_socket/test_completion_callback.h"
#include "mojo/shell/domain_socket/unix_domain_server_socket_posix.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace mojo {
namespace shell {
namespace {

const char kSocketFilename[] = "socket_for_testing";

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

// Connects socket synchronously.
int ConnectSynchronously(UnixDomainClientSocket* socket) {
  TestCompletionCallback connect_callback;
  int rv = socket->Connect(connect_callback.callback());
  if (rv == net::ERR_IO_PENDING)
    rv = connect_callback.WaitForResult();
  return rv;
}
}  // namespace

class UnixDomainClientSocketTest : public testing::Test {
 protected:
  UnixDomainClientSocketTest() {
    EXPECT_TRUE(temp_dir_.CreateUniqueTempDir());
    socket_path_ = temp_dir_.path().Append(kSocketFilename).value();
  }

  base::ScopedTempDir temp_dir_;
  std::string socket_path_;
  base::MessageLoopForIO loop_;
};

TEST_F(UnixDomainClientSocketTest, ConnectWithSocketDescriptor) {
  const bool kUseAbstractNamespace = false;

  UnixDomainServerSocket server_socket(CreateAuthCallback(true),
                                       kUseAbstractNamespace);
  EXPECT_EQ(net::OK, server_socket.ListenWithPath(socket_path_, 1));

  SocketDescriptor accepted_socket_fd = kInvalidSocket;
  TestCompletionCallback accept_callback;
  EXPECT_EQ(
      net::ERR_IO_PENDING,
      server_socket.Accept(&accepted_socket_fd, accept_callback.callback()));
  EXPECT_EQ(kInvalidSocket, accepted_socket_fd);

  UnixDomainClientSocket client_socket(socket_path_, kUseAbstractNamespace);
  EXPECT_FALSE(client_socket.IsConnected());

  EXPECT_EQ(net::OK, ConnectSynchronously(&client_socket));
  EXPECT_TRUE(client_socket.IsConnected());
  // Server has not yet been notified of the connection.
  EXPECT_EQ(kInvalidSocket, accepted_socket_fd);

  EXPECT_EQ(net::OK, accept_callback.WaitForResult());
  EXPECT_NE(kInvalidSocket, accepted_socket_fd);

  SocketDescriptor client_socket_fd = client_socket.ReleaseConnectedSocket();
  EXPECT_NE(kInvalidSocket, client_socket_fd);

  // Now, re-wrap client_socket_fd in a UnixDomainClientSocket and check that
  // it hasn't gotten accidentally closed.
  SockaddrStorage addr;
  ASSERT_TRUE(UnixDomainClientSocket::FillAddress(socket_path_, false, &addr));
  scoped_ptr<SocketLibevent> adopter(new SocketLibevent);
  adopter->AdoptConnectedSocket(client_socket_fd, addr);
  UnixDomainClientSocket rewrapped_socket(adopter.Pass());
  EXPECT_TRUE(rewrapped_socket.IsConnected());

  EXPECT_EQ(0, IGNORE_EINTR(close(accepted_socket_fd)));
}

TEST_F(UnixDomainClientSocketTest, ConnectWithAbstractNamespace) {
  const bool kUseAbstractNamespace = true;

  UnixDomainClientSocket client_socket(socket_path_, kUseAbstractNamespace);
  EXPECT_FALSE(client_socket.IsConnected());

  UnixDomainServerSocket server_socket(CreateAuthCallback(true),
                                       kUseAbstractNamespace);
  EXPECT_EQ(net::OK, server_socket.ListenWithPath(socket_path_, 1));

  SocketDescriptor accepted_socket_fd = kInvalidSocket;
  TestCompletionCallback accept_callback;
  EXPECT_EQ(
      net::ERR_IO_PENDING,
      server_socket.Accept(&accepted_socket_fd, accept_callback.callback()));
  EXPECT_EQ(kInvalidSocket, accepted_socket_fd);

  EXPECT_EQ(net::OK, ConnectSynchronously(&client_socket));
  EXPECT_TRUE(client_socket.IsConnected());
  // Server has not yet been notified of the connection.
  EXPECT_EQ(kInvalidSocket, accepted_socket_fd);

  EXPECT_EQ(net::OK, accept_callback.WaitForResult());
  EXPECT_NE(kInvalidSocket, accepted_socket_fd);

  EXPECT_EQ(0, IGNORE_EINTR(close(accepted_socket_fd)));
}

TEST_F(UnixDomainClientSocketTest, ConnectToNonExistentSocket) {
  const bool kUseAbstractNamespace = false;

  UnixDomainClientSocket client_socket(socket_path_, kUseAbstractNamespace);
  EXPECT_FALSE(client_socket.IsConnected());
  EXPECT_EQ(net::ERR_FILE_NOT_FOUND, ConnectSynchronously(&client_socket));
}

TEST_F(UnixDomainClientSocketTest,
       ConnectToNonExistentSocketWithAbstractNamespace) {
  const bool kUseAbstractNamespace = true;

  UnixDomainClientSocket client_socket(socket_path_, kUseAbstractNamespace);
  EXPECT_FALSE(client_socket.IsConnected());

  TestCompletionCallback connect_callback;
  EXPECT_EQ(net::ERR_CONNECTION_REFUSED, ConnectSynchronously(&client_socket));
}

}  // namespace shell
}  // namespace mojo
