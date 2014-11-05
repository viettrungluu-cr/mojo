// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_HTTP_SERVER_HTTP_REQUEST_PARSER_H_
#define SERVICES_HTTP_SERVER_HTTP_REQUEST_PARSER_H_

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "services/http_server/public/http_request.mojom.h"

namespace mojo {

// Parses the input data and produces a valid HttpRequest object. If there is
// more than one request in one chunk, then only the first one will be parsed.
class HttpRequestParser {
 public:
  // Parsing result.
  enum ParseResult {
    WAITING,  // A request is not completed yet, waiting for more data.
    ACCEPTED,  // A request has been parsed and it is ready to be processed.
    ERROR,  // There was an error parsing the request.
  };

  HttpRequestParser();
  ~HttpRequestParser();

  // Adds chunk of data into the internal buffer.
  void ProcessChunk(const base::StringPiece& data);

  // Parses the http request (including data - if provided).
  // If returns ACCEPTED, then it means that the whole request has been found
  // in the internal buffer (and parsed).
  ParseResult ParseRequest();

  // Retrieves parsed request. Can be only called when the parser is in
  // STATE_ACCEPTED state.
  HttpRequestPtr GetRequest();

 private:
  // Parser state.
  enum State {
    STATE_HEADERS,  // Waiting for a request headers.
    STATE_CONTENT,  // Waiting for content data.
    STATE_ACCEPTED,  // Request has been parsed.
  };

  // Parses headers and returns ACCEPTED if whole request was parsed. Otherwise
  // returns WAITING.
  ParseResult ParseHeaders();

  // Parses request's content data and returns ACCEPTED if all of it have been
  // processed. Chunked Transfer Encoding *is not* supported.
  ParseResult ParseContent();

  // Fetches the next line from the buffer. Result does not contain \r\n.
  // Returns an empty string for an empty line. It will assert if there is
  // no line available.
  std::string ShiftLine();

  // Returns the number of bytes in |http_request_->body|.
  uint32_t GetBodySize();

  HttpRequestPtr http_request_;
  ScopedDataPipeProducerHandle producer_handle_;
  std::string buffer_;
  size_t buffer_position_;  // Current position in the internal buffer.
  State state_;
  // Content length of the request currently being parsed.
  size_t declared_content_length_;

  DISALLOW_COPY_AND_ASSIGN(HttpRequestParser);
};

}  // namespace mojo

#endif  // SERVICES_HTTP_SERVER_HTTP_REQUEST_PARSER_H_
