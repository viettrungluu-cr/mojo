// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:mojo/dart/testing/expect.dart';
import 'package:mojo/public/dart/core.dart';
import 'package:mojo/public/dart/mojo_init.dart';


void simpleTest() {
  var pipe = new MojoMessagePipe();
  Expect.isNotNull(pipe);

  var endpoint = pipe.endpoints[0];
  Expect.notEquals(endpoint.handle, RawMojoHandle.INVALID);

  var handle = new MojoHandle(endpoint.handle);
  var completer = new Completer();
  int numEvents = 0;

  handle.enableWriteEvents();
  handle.listen((_) {
    numEvents++;
    handle.close();
  }, onDone: () {
    completer.complete(numEvents);
  });

  completer.future.then((int numEvents) {
    Expect.equals(1, numEvents);
  });
}


Future simpleAsyncAwaitTest() async {
  var pipe = new MojoMessagePipe();
  Expect.isNotNull(pipe);

  var endpoint = pipe.endpoints[0];
  Expect.notEquals(endpoint.handle, RawMojoHandle.INVALID);

  var handle = new MojoHandle(endpoint.handle);

  int numEvents = 0;
  handle.enableWriteEvents();
  await for (var signal in handle) {
    numEvents++;
    handle.close();
  }
  Expect.equals(1, numEvents);
}


ByteData byteDataOfString(String s) {
  return new ByteData.view((new Uint8List.fromList(s.codeUnits)).buffer);
}


String stringOfByteData(ByteData bytes) {
  return new String.fromCharCodes(bytes.buffer.asUint8List().toList());
}


void expectStringFromEndpoint(String expected,
                              MojoMessagePipeEndpoint endpoint) {
  // Query how many bytes are available.
  var result = endpoint.query();
  Expect.isNotNull(result);
  int size = result.bytesRead;
  Expect.isTrue(size > 0);

  // Read the data.
  ByteData bytes = new ByteData(size);
  result = endpoint.read(bytes);
  Expect.isNotNull(result);
  Expect.equals(size, result.bytesRead);

  // Convert to a string and check.
  String msg = stringOfByteData(bytes);
  Expect.equals(expected, msg);
}


Future pingPongIsolate(MojoMessagePipeEndpoint endpoint) async {
  int pings = 0;
  int pongs = 0;
  var handle = new MojoHandle(endpoint.handle);
  await for (var signal in handle) {
    if (MojoHandleSignals.isReadWrite(signal)) {
      // We are either sending or receiving.
      throw new Exception("Unexpected signal");
    } else if (MojoHandleSignals.isReadable(signal)) {
      expectStringFromEndpoint("Ping", endpoint);
      pings++;
      handle.enableWriteEvents();
    } else if (MojoHandleSignals.isWritable(signal)) {
      endpoint.write(byteDataOfString("Pong"));
      pongs++;
      handle.disableWriteEvents();
    }
  }
  handle.close();
  Expect.equals(10, pings);
  Expect.equals(10, pongs);
}


Future pingPongTest() async {
  var pipe = new MojoMessagePipe();
  var isolate = await Isolate.spawn(pingPongIsolate, pipe.endpoints[0]);
  var endpoint = pipe.endpoints[1];
  var handle = new MojoHandle(endpoint.handle);

  int pings = 0;
  int pongs = 0;
  handle.enableWriteEvents();  // This side will send first.
  await for (var signal in handle) {
    if (MojoHandleSignals.isReadWrite(signal)) {
      // We are either sending or receiving.
      throw new Exception("Unexpected signal");
    } else if (MojoHandleSignals.isReadable(signal)) {
      expectStringFromEndpoint("Pong", endpoint);
      pongs++;
      if (pongs == 10) {
        handle.close();
      }
      handle.enableWriteEvents();  // Now it is our turn to send.
    } else if (MojoHandleSignals.isWritable(signal)) {
      if (pings < 10) {
        endpoint.write(byteDataOfString("Ping"));
        pings++;
      }
      handle.disableWriteEvents();  // Don't send while waiting for reply.
    }
  }
  Expect.equals(10, pings);
  Expect.equals(10, pongs);
}


main() async {
  await mojoInit();
  simpleTest();
  await simpleAsyncAwaitTest();
  await pingPongTest();
}
