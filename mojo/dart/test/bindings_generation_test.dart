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
import 'package:mojo/public/interfaces/bindings/tests/sample_interfaces.mojom.dart' as sample;


void providerIsolate(core.MojoMessagePipeEndpoint endpoint) {
  var provider = new sample.ProviderInterface(endpoint);
  provider.listen((msg) {
    if (msg is sample.Provider_EchoString_Params) {
      var response = new sample.Provider_EchoString_ResponseParams();
      response.a = msg.a;
      return response;
    } else if (msg is sample.Provider_EchoStrings_Params) {
      var response = new sample.Provider_EchoStrings_ResponseParams();
      response.a = msg.a;
      response.b = msg.b;
      return response;
    } else if (msg is sample.Provider_EchoMessagePipeHandle_Params) {
      var response = new sample.Provider_EchoMessagePipeHandle_ResponseParams();
      response.a = msg.a;
      return response;
    } else if (msg is sample.Provider_EchoEnum_Params) {
      var response = new sample.Provider_EchoEnum_ResponseParams();
      response.a = msg.a;
      return response;
    } else {
      throw new Exception("Unexpected message");
    }
  });
}


Future<bool> test() {
  var pipe = new core.MojoMessagePipe();
  var client = new sample.ProviderClient(pipe.endpoints[0]);
  var c = new Completer();
  client.open();
  Isolate.spawn(providerIsolate, pipe.endpoints[1]).then((_) {
    client.echoString("hello!").then((echoStringResponse) {
      Expect.equals("hello!", echoStringResponse.a);
    }).then((_) {
      client.echoStrings("hello", "mojo!").then((echoStringsResponse) {
        Expect.equals("hello", echoStringsResponse.a);
        Expect.equals("mojo!", echoStringsResponse.b);
        client.close();
        c.complete(true);
      });
    });
  });
  return c.future;
}


Future testAwait() async {
  var pipe = new core.MojoMessagePipe();
  var client = new sample.ProviderClient(pipe.endpoints[0]);
  var isolate = await Isolate.spawn(providerIsolate, pipe.endpoints[1]);

  client.open();
  var echoStringResponse = await client.echoString("hello!");
  Expect.equals("hello!", echoStringResponse.a);

  var echoStringsResponse = await client.echoStrings("hello", "mojo!");
  Expect.equals("hello", echoStringsResponse.a);
  Expect.equals("mojo!", echoStringsResponse.b);

  client.close();
}


main() async {
  await init.mojoInit();
  await test();
  await testAwait();
  init.mojoShutdown();
}
