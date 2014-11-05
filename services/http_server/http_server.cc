// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>

#if defined(OS_WIN)
#include <winsock2.h>
#elif defined(OS_POSIX)
#include <arpa/inet.h>
#endif

#include "base/bind.h"
#include "base/format_macros.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/stringprintf.h"
#include "mojo/public/c/system/main.h"
#include "mojo/public/cpp/application/application_delegate.h"
#include "mojo/public/cpp/application/application_impl.h"
#include "mojo/public/cpp/application/application_runner.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/utility/async_waiter.h"
#include "mojo/services/public/interfaces/network/network_service.mojom.h"
#include "services/http_server/http_request_parser.h"
#include "services/http_server/public/http_request.mojom.h"
#include "services/http_server/public/http_response.mojom.h"
#include "third_party/re2/re2/re2.h"

namespace mojo {
namespace examples {

class Connection;

typedef base::Callback<HttpResponsePtr(
    const HttpRequest& request)> HandleRequestCallback;

const char* GetHttpReasonPhrase(uint32_t code_in) {
  switch (code_in) {
#define HTTP_STATUS(label, code, reason) case code: return reason;
#include "net/http/http_status_code_list.h"
#undef HTTP_STATUS

    default:
      NOTREACHED() << "unknown HTTP status code " << code_in;
  }

  return "";
}

// Represents one connection to a client. This connection will manage its own
// lifetime and will delete itself when the connection is closed.
class Connection {
 public:
  // Callback called when a request is parsed. Response should be sent
  // using Connection::SendResponse() on the |connection| argument.
  typedef base::Callback<void(Connection*, const HttpRequest&)> Callback;

  Connection(TCPConnectedSocketPtr conn,
             ScopedDataPipeProducerHandle sender,
             ScopedDataPipeConsumerHandle receiver,
             const Callback& callback)
      : connection_(conn.Pass()),
        sender_(sender.Pass()),
        receiver_(receiver.Pass()),
        request_waiter_(receiver_.get(), MOJO_HANDLE_SIGNAL_READABLE,
                        base::Bind(&Connection::OnRequestDataReady,
                                   base::Unretained(this))),
        content_length_(0),
        handle_request_callback_(callback),
        response_offset_(0) {
  }

  ~Connection() {
  }

  void SendResponse(HttpResponsePtr response) {
    std::string http_reason_phrase(GetHttpReasonPhrase(response->status_code));

    // TODO(mtomasz): For http/1.0 requests, send http/1.0.
    base::StringAppendF(&response_,
                        "HTTP/1.1 %d %s\r\n",
                        response->status_code,
                        http_reason_phrase.c_str());
    base::StringAppendF(&response_, "Connection: close\r\n");

    content_length_ = response->content_length;
    if (content_length_) {
      base::StringAppendF(&response_,
                          "Content-Length: %" PRIuS "\r\n",
                          static_cast<size_t>(content_length_));
    }
    base::StringAppendF(&response_,
                        "Content-Type: %s\r\n",
                        response->content_type.data());
    for (auto it = response->custom_headers.begin();
         it != response->custom_headers.end(); ++it) {
      const std::string& header_name = it.GetValue();
      const std::string& header_value = it.GetKey();
      DCHECK(header_value.find_first_of("\n\r") == std::string::npos) <<
          "Malformed header value.";
      base::StringAppendF(&response_,
                          "%s: %s\r\n",
                          header_name.c_str(),
                          header_value.c_str());
    }
    base::StringAppendF(&response_, "\r\n");

    content_ = response->body.Pass();
    WriteMore();
  }

 private:
  // Called when we have more data available from the request.
  void OnRequestDataReady(MojoResult result) {
    uint32_t num_bytes = 0;
    result = ReadDataRaw(receiver_.get(), NULL, &num_bytes,
                         MOJO_READ_DATA_FLAG_QUERY);
    printf("ReadDataRaw result = %d, num_bytes = %d\n", result, num_bytes);
    if (!num_bytes)
      return;

    scoped_ptr<uint8_t[]> buffer(new uint8_t[num_bytes]);
    result = ReadDataRaw(receiver_.get(), buffer.get(), &num_bytes,
                         MOJO_READ_DATA_FLAG_ALL_OR_NONE);

    request_parser_.ProcessChunk(reinterpret_cast<char*>(buffer.get()));
    if (request_parser_.ParseRequest() == HttpRequestParser::ACCEPTED) {
      HttpRequestPtr http_request = request_parser_.GetRequest();
      handle_request_callback_.Run(this, *http_request.get());
    }
  }

  void WriteMore() {
    uint32_t response_bytes_available =
        static_cast<uint32_t>(response_.size() - response_offset_);
    if (response_bytes_available) {
      MojoResult result = WriteDataRaw(
          sender_.get(), &response_[response_offset_],
          &response_bytes_available, 0);
      if (result == MOJO_RESULT_SHOULD_WAIT) {
        sender_waiter_.reset(new AsyncWaiter(
            sender_.get(), MOJO_HANDLE_SIGNAL_WRITABLE,
            base::Bind(&Connection::OnSenderReady, base::Unretained(this))));
        return;
      } else if (result != MOJO_RESULT_OK) {
        printf("Error writing to pipe.\n");
        delete this;
        return;
      }

      response_offset_ += response_bytes_available;
    }

    if (response_offset_ != response_.size()) {
      // We have more data left in response_. Write more asynchronously.
      sender_waiter_.reset(new AsyncWaiter(
            sender_.get(), MOJO_HANDLE_SIGNAL_WRITABLE,
            base::Bind(&Connection::OnSenderReady, base::Unretained(this))));
      return;
    }

    // response_ is all sent, and there's no more data so we're done.
    if (!content_length_) {
      delete this;
      return;
    }

    // Copy data from the handler's pipe to response_.
    const uint32_t kMaxChunkSize = 1024 * 1024;

    uint32_t num_bytes_available = 0;
    MojoResult result = ReadDataRaw(content_.get(), NULL,
                                    &num_bytes_available,
                                    MOJO_READ_DATA_FLAG_QUERY);
    if (result == MOJO_RESULT_SHOULD_WAIT) {
      // Producer isn't ready yet. Wait for it.
      response_receiver_waiter_.reset(new AsyncWaiter(
          content_.get(), MOJO_HANDLE_SIGNAL_READABLE,
          base::Bind(&Connection::OnResponseDataReady,
                     base::Unretained(this))));
      return;
    }

    DCHECK_EQ(result, MOJO_RESULT_OK);
    num_bytes_available = std::min(num_bytes_available, kMaxChunkSize);

    response_.resize(num_bytes_available);
    response_offset_ = 0;
    content_length_ -= num_bytes_available;

    result = ReadDataRaw(content_.get(), &response_[0],
                         &num_bytes_available,
                         MOJO_READ_DATA_FLAG_ALL_OR_NONE);
    DCHECK_EQ(result, MOJO_RESULT_OK);
    sender_waiter_.reset(new AsyncWaiter(
        sender_.get(), MOJO_HANDLE_SIGNAL_WRITABLE,
        base::Bind(&Connection::OnSenderReady, base::Unretained(this))));
  }

  void OnResponseDataReady(MojoResult result) {
    DCHECK_EQ(result, MOJO_RESULT_OK);
    WriteMore();
  }

  void OnSenderReady(MojoResult result) {
    DCHECK_EQ(result, MOJO_RESULT_OK);
    WriteMore();
  }

  TCPConnectedSocketPtr connection_;
  ScopedDataPipeProducerHandle sender_;
  ScopedDataPipeConsumerHandle receiver_;

  // Used to wait for the request data.
  AsyncWaiter request_waiter_;

  int content_length_;
  ScopedDataPipeConsumerHandle content_;

  // Used to wait for the response data to send.
  scoped_ptr<AsyncWaiter> response_receiver_waiter_;

  // Used to wait for the sender to be ready to accept more data.
  scoped_ptr<AsyncWaiter> sender_waiter_;

  HttpRequestParser request_parser_;

  // Callback to run once all of the request has been read.
  const Callback handle_request_callback_;

  // Contains response data to write to the pipe. Initially it is the headers,
  // and then when they're written it contains chunks of the body.
  std::string response_;
  size_t response_offset_;
};

HttpResponsePtr CreateResponse(int code, const std::string& data) {
  HttpResponsePtr response = HttpResponse::New();

  ScopedDataPipeProducerHandle producer_handle;
  MojoCreateDataPipeOptions options = {sizeof(MojoCreateDataPipeOptions),
                                       MOJO_CREATE_DATA_PIPE_OPTIONS_FLAG_NONE,
                                       1,
                                       data.size()};
  MojoResult result = CreateDataPipe(
      &options, &producer_handle, &response->body);
  DCHECK_EQ(MOJO_RESULT_OK, result);
  uint32_t num_bytes = data.size();
  result = WriteDataRaw(
      producer_handle.get(), data.c_str(), &num_bytes,
      MOJO_WRITE_DATA_FLAG_ALL_OR_NONE);
  DCHECK_EQ(MOJO_RESULT_OK, result);
  response->content_length = num_bytes;
  return response.Pass();
}

HttpResponsePtr FooHandler(const HttpRequest& request) {
  return CreateResponse(200, "Foo").Pass();
}

HttpResponsePtr BarHandler(const HttpRequest& request) {
  return CreateResponse(200, "Bar").Pass();
}

class HttpServerApp : public ApplicationDelegate {
 public:
  HttpServerApp() : weak_ptr_factory_(this) {}
  virtual void Initialize(ApplicationImpl* app) override {
    app->ConnectToService("mojo:network_service", &network_service_);

    AddHandler("/foo", base::Bind(FooHandler));
    AddHandler("/bar", base::Bind(BarHandler));

    Start();
  }

  // Add a handler for the given regex path.
  void AddHandler(const std::string& path,
                  const HandleRequestCallback& handler) {
    handlers_.push_back(Handler(path, handler));
  }

 private:
  void OnSocketBound(NetworkErrorPtr err, NetAddressPtr bound_address) {
    if (err->code != 0) {
      printf("Bound err = %d\n", err->code);
      return;
    }

    printf("Got address %d.%d.%d.%d:%d\n",
           (int)bound_address->ipv4->addr[0],
           (int)bound_address->ipv4->addr[1],
           (int)bound_address->ipv4->addr[2],
           (int)bound_address->ipv4->addr[3],
           (int)bound_address->ipv4->port);
  }

  void OnSocketListening(NetworkErrorPtr err) {
    if (err->code != 0) {
      printf("Listen err = %d\n", err->code);
      return;
    }
    printf("Waiting for incoming connections...\n");
  }

  void OnConnectionAccepted(NetworkErrorPtr err, NetAddressPtr remote_address) {
    if (err->code != 0) {
      printf("Accepted socket error = %d\n", err->code);
      return;
    }

    new Connection(pending_connected_socket_.Pass(),
                   pending_send_handle_.Pass(),
                   pending_receive_handle_.Pass(),
                   base::Bind(&HttpServerApp::HandleRequest,
                              weak_ptr_factory_.GetWeakPtr()));

    // Ready for another connection.
    WaitForNextConnection();
  }

  void WaitForNextConnection() {
    // Need two pipes (one for each direction).
    ScopedDataPipeConsumerHandle send_consumer_handle;
    MojoResult result = CreateDataPipe(
        nullptr, &pending_send_handle_, &send_consumer_handle);
    assert(result == MOJO_RESULT_OK);

    ScopedDataPipeProducerHandle receive_producer_handle;
    result = CreateDataPipe(
        nullptr, &receive_producer_handle, &pending_receive_handle_);
    assert(result == MOJO_RESULT_OK);
    MOJO_ALLOW_UNUSED_LOCAL(result);

    server_socket_->Accept(send_consumer_handle.Pass(),
                           receive_producer_handle.Pass(),
                           GetProxy(&pending_connected_socket_),
                           base::Bind(&HttpServerApp::OnConnectionAccepted,
                                      base::Unretained(this)));
  }

  void Start() {
    NetAddressPtr net_address(NetAddress::New());
    net_address->family = NET_ADDRESS_FAMILY_IPV4;
    net_address->ipv4 = NetAddressIPv4::New();
    net_address->ipv4->addr.resize(4);
    net_address->ipv4->addr[0] = 127;
    net_address->ipv4->addr[1] = 0;
    net_address->ipv4->addr[2] = 0;
    net_address->ipv4->addr[3] = 1;
    net_address->ipv4->port = 0;

    // Note that we can start using the proxies right away even thought the
    // callbacks have not been called yet. If a previous step fails, they'll
    // all fail.
    network_service_->CreateTCPBoundSocket(
        net_address.Pass(),
        GetProxy(&bound_socket_),
        base::Bind(&HttpServerApp::OnSocketBound, base::Unretained(this)));
    bound_socket_->StartListening(GetProxy(
        &server_socket_),
        base::Bind(&HttpServerApp::OnSocketListening, base::Unretained(this)));
    WaitForNextConnection();
  }

  void HandleRequest(Connection* connection, const HttpRequest& request) {
    printf("HandleRequest for %s\n", request.relative_url.data());
    for (auto& handler : handlers_) {
      if (RE2::FullMatch(request.relative_url.data(), handler.pattern)) {
        connection->SendResponse(handler.callback.Run(request).Pass());
        return;
      }
    }

    connection->SendResponse(
        CreateResponse(404, "No registered handler").Pass());
  }

  struct Handler {
    Handler(const std::string& pattern,
            const HandleRequestCallback& callback)
        : pattern(pattern.c_str()), callback(callback) {}
    Handler(const Handler& handler)
        : pattern(handler.pattern.pattern()), callback(handler.callback) {}
    RE2 pattern;
    HandleRequestCallback callback;
  };

  base::WeakPtrFactory<HttpServerApp> weak_ptr_factory_;

  NetworkServicePtr network_service_;
  TCPBoundSocketPtr bound_socket_;
  TCPServerSocketPtr server_socket_;

  ScopedDataPipeProducerHandle pending_send_handle_;
  ScopedDataPipeConsumerHandle pending_receive_handle_;
  TCPConnectedSocketPtr pending_connected_socket_;

  std::vector<Handler> handlers_;
};

}  // namespace examples
}  // namespace mojo

MojoResult MojoMain(MojoHandle shell_handle) {
  mojo::ApplicationRunner runner(new mojo::examples::HttpServerApp);
  return runner.Run(shell_handle);
}
