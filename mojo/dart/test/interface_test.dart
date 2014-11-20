// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:mojo/dart/testing/expect.dart';
import 'package:mojo/public/dart/bindings.dart' as bindings;
import 'package:mojo/public/dart/core.dart' as core;
import 'package:mojo/public/dart/mojo_init.dart' as init;

class EchoString implements bindings.MojoType<EchoString> {
  String a = null;

  EchoString();

  static const int encodedSize = bindings.kStructHeaderSize + 8;
  static EchoString decode(bindings.MojoDecoder decoder) {
    var val = new EchoString();
    var numBytes = decoder.readUint32();
    var numFields = decoder.readUint32();
    val.a = decoder.decodeStruct(bindings.MojoString);
    return val;
  }
  static void encode(bindings.MojoEncoder encoder, EchoString val) {
    encoder.writeUint32(encodedSize);
    encoder.writeUint32(1);
    encoder.encodeStruct(bindings.MojoString, val.a);
  }
}


class EchoStringResponse extends EchoString {
  static const int encodedSize = EchoString.encodedSize;
  static var decode = EchoString.decode;
  static var encode = EchoString.encode;
}


const int kEchoString_name = 0;
const int kEchoStringResponse_name = 1;

class EchoInterface extends bindings.Interface {
  EchoInterface(core.MojoMessagePipeEndpoint endpoint) : super(endpoint);

  bindings.Message handleMessage(bindings.MessageReader reader,
                                 Function messageHandler) {
    switch (reader.name) {
      case kEchoString_name:
        var es = reader.decodeStruct(EchoString);
        var response = messageHandler(es);
        return buildResponseWithID(EchoStringResponse,
                                   kEchoStringResponse_name,
                                   bindings.kMessageIsResponse,
                                   response);
        break;
      default:
        throw new Exception("Unexpected case");
        break;
    }
    return null;
  }
}


class EchoClient extends bindings.Client {
  EchoClient(core.MojoMessagePipeEndpoint endpoint) : super(endpoint);

  Future<EchoStringResponse> echoString(String a) {
    // compose message.
    var es = new EchoString();
    es.a = a;
    return enqueueMessageWithRequestID(EchoString,
                                       kEchoString_name,
                                       bindings.kMessageExpectsResponse,
                                       es);
  }

  void handleResponse(bindings.MessageReader reader) {
    switch (reader.name) {
      case kEchoStringResponse_name:
        var esr = reader.decodeStruct(EchoStringResponse);
        Completer c = completerQueue.removeAt(0);
        c.complete(esr);
        break;
      default:
        throw new Exception("Unexpected case");
        break;
    }
  }
}


void providerIsolate(core.MojoMessagePipeEndpoint endpoint) {
  var provider = new EchoInterface(endpoint);
  provider.listen((msg) {
    if (msg is EchoString) {
      var response = new EchoStringResponse();
      response.a = msg.a;
      return response;
    }
  });
}


Future<bool> runTest() async {
  var testCompleter = new Completer();

  var pipe = new core.MojoMessagePipe();
  var client = new EchoClient(pipe.endpoints[0]);
  await Isolate.spawn(providerIsolate, pipe.endpoints[1]);

  int n = 100;
  int count = 0;
  client.open();
  for (int i = 0; i < n; i++) {
    client.echoString("hello").then((response) {
      Expect.equals("hello", response.a);
      count++;
      if (i == (n - 1)) {
        client.close();
        testCompleter.complete(count);
      }
    });
  }

  return testCompleter.future;
}


Future runAwaitTest() async {
  var pipe = new core.MojoMessagePipe();
  var client = new EchoClient(pipe.endpoints[0]);
  await Isolate.spawn(providerIsolate, pipe.endpoints[1]);

  int n = 100;
  client.open();
  for (int i = 0; i < n; i++) {
    var response = await client.echoString("Hello");
    Expect.equals("Hello", response.a);
  }
  client.close();
}


main() async {
  await init.mojoInit();
  Expect.equals(100, await runTest());
  await runAwaitTest();
  init.mojoShutdown();
}
