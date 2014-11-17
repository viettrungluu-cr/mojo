// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:mojo/dart/testing/expect.dart';
import 'package:mojo/public/dart/core.dart';
import 'package:mojo/public/dart/mojo_init.dart';


invalidHandleTest() {
  RawMojoHandle invalidHandle = new RawMojoHandle(RawMojoHandle.INVALID);

  // Close.
  MojoResult result = invalidHandle.close();
  Expect.isTrue(result.isInvalidArgument);

  // Wait.
  result = invalidHandle.wait(MojoHandleSignals.READWRITE, 1000000);
  Expect.isTrue(result.isInvalidArgument);

  int res = RawMojoHandle.waitMany([invalidHandle.h],
                                   [MojoHandleSignals.READWRITE],
                                   RawMojoHandle.DEADLINE_INDEFINITE);
  Expect.equals(res, MojoResult.kInvalidArgument);

  // Message pipe.
  MojoMessagePipe pipe = new MojoMessagePipe();
  Expect.isNotNull(pipe);
  ByteData bd = new ByteData(10);
  pipe.endpoints[0].handle.close();
  pipe.endpoints[1].handle.close();
  result = pipe.endpoints[0].write(bd);
  Expect.isTrue(result.isInvalidArgument);

  MojoMessagePipeReadResult readResult = pipe.endpoints[0].read(bd);
  Expect.isTrue(pipe.endpoints[0].status.isInvalidArgument);

  // Data pipe.
  MojoDataPipe dataPipe = new MojoDataPipe();
  Expect.isNotNull(dataPipe);
  dataPipe.producer.handle.close();
  dataPipe.consumer.handle.close();

  int bytesWritten = dataPipe.producer.write(bd);
  Expect.isTrue(dataPipe.producer.status.isInvalidArgument);

  ByteData writeData = dataPipe.producer.beginWrite(10);
  Expect.isNull(writeData);
  Expect.isTrue(dataPipe.producer.status.isInvalidArgument);
  dataPipe.producer.endWrite(10);
  Expect.isTrue(dataPipe.producer.status.isInvalidArgument);

  int read = dataPipe.consumer.read(bd);
  Expect.isTrue(dataPipe.consumer.status.isInvalidArgument);

  ByteData readData = dataPipe.consumer.beginRead(10);
  Expect.isNull(readData);
  Expect.isTrue(dataPipe.consumer.status.isInvalidArgument);
  dataPipe.consumer.endRead(10);
  Expect.isTrue(dataPipe.consumer.status.isInvalidArgument);

  // Shared buffer.
  MojoSharedBuffer sharedBuffer = new MojoSharedBuffer(10);
  Expect.isNotNull(sharedBuffer);
  sharedBuffer.close();
  MojoSharedBuffer duplicate = new MojoSharedBuffer.duplicate(sharedBuffer);
  Expect.isNull(duplicate);

  sharedBuffer = new MojoSharedBuffer(10);
  Expect.isNotNull(sharedBuffer);
  sharedBuffer.close();
  result = sharedBuffer.map(0, 10);
  Expect.isTrue(result.isInvalidArgument);
}


basicMessagePipeTest() {
  MojoMessagePipe pipe = new MojoMessagePipe();
  Expect.isNotNull(pipe);
  Expect.isTrue(pipe.status.isOk);
  Expect.isNotNull(pipe.endpoints);

  MojoMessagePipeEndpoint end0 = pipe.endpoints[0];
  MojoMessagePipeEndpoint end1 = pipe.endpoints[1];
  Expect.isTrue(RawMojoHandle.isValid(end0.handle));
  Expect.isTrue(RawMojoHandle.isValid(end1.handle));

  // Not readable, yet.
  MojoResult result = end0.handle.wait(MojoHandleSignals.READABLE, 0);
  Expect.isTrue(result.isDeadlineExceeded);

  // Should be writable.
  result = end0.handle.wait(MojoHandleSignals.WRITABLE, 0);
  Expect.isTrue(result.isOk);

  // Try to read.
  ByteData data = new ByteData(10);
  end0.read(data);
  Expect.isTrue(end0.status.isShouldWait);

  // Write end1.
  String hello = "hello";
  ByteData helloData =
      new ByteData.view((new Uint8List.fromList(hello.codeUnits)).buffer);
  result = end1.write(helloData);
  Expect.isTrue(result.isOk);

  // end0 should now be readable.
  int res = RawMojoHandle.waitMany([end0.handle.h],
                                   [MojoHandleSignals.READABLE],
                                   RawMojoHandle.DEADLINE_INDEFINITE);
  Expect.equals(res, MojoResult.kOk);

  // Read from end0.
  MojoMessagePipeReadResult readResult = end0.read(data);
  Expect.isNotNull(readResult);
  Expect.isTrue(readResult.status.isOk);
  Expect.equals(readResult.bytesRead, helloData.lengthInBytes);
  Expect.equals(readResult.handlesRead, 0);

  String hello_result = new String.fromCharCodes(
      data.buffer.asUint8List().sublist(0, readResult.bytesRead).toList());
  Expect.equals(hello_result, "hello");

  // end0 should no longer be readable.
  result = end0.handle.wait(MojoHandleSignals.READABLE, 10);
  Expect.isTrue(result.isDeadlineExceeded);

  // Close end0's handle.
  result = end0.handle.close();
  Expect.isTrue(result.isOk);

  // end1 should no longer be readable or writable.
  result = end1.handle.wait(MojoHandleSignals.READWRITE, 1000);
  Expect.isTrue(result.isFailedPrecondition);

  result = end1.handle.close();
  Expect.isTrue(result.isOk);
}


basicDataPipeTest() {
  MojoDataPipe pipe = new MojoDataPipe();
  Expect.isNotNull(pipe);
  Expect.isTrue(pipe.status.isOk);
  Expect.notEquals(pipe.consumer.handle, RawMojoHandle.INVALID);
  Expect.notEquals(pipe.producer.handle, RawMojoHandle.INVALID);

  MojoDataPipeProducer producer = pipe.producer;
  MojoDataPipeConsumer consumer = pipe.consumer;
  Expect.notEquals(producer.handle.h, RawMojoHandle.INVALID);
  Expect.notEquals(consumer.handle.h, RawMojoHandle.INVALID);

  // Consumer should not be readable.
  MojoResult result = consumer.handle.wait(MojoHandleSignals.READABLE, 0);
  Expect.isTrue(result.isDeadlineExceeded);

  // Producer should be writable.
  result = producer.handle.wait(MojoHandleSignals.WRITABLE, 0);
  Expect.isTrue(result.isOk);

  // Try to read from consumer.
  ByteData buffer = new ByteData(20);
  consumer.read(buffer, buffer.lengthInBytes, MojoDataPipeConsumer.FLAG_NONE);
  Expect.isTrue(consumer.status.isShouldWait);

  // Try to begin a two-phase read from consumer.
  ByteData b = consumer.beginRead(20, MojoDataPipeConsumer.FLAG_NONE);
  Expect.isNull(b);
  Expect.isTrue(consumer.status.isShouldWait);

  // Write to producer.
  String hello = "hello ";
  ByteData helloData =
      new ByteData.view((new Uint8List.fromList(hello.codeUnits)).buffer);
  int written = producer.write(
      helloData, helloData.lengthInBytes, MojoDataPipeProducer.FLAG_NONE);
  Expect.isTrue(producer.status.isOk);
  Expect.equals(written, helloData.lengthInBytes);

  // Now that we have written, the consumer should be readable.
  int res = RawMojoHandle.waitMany([consumer.handle.h],
                                   [MojoHandleSignals.READABLE],
                                   RawMojoHandle.DEADLINE_INDEFINITE);
  Expect.equals(res, MojoResult.kOk);

  // Do a two-phase write to the producer.
  ByteData twoPhaseWrite = producer.beginWrite(
      20, MojoDataPipeProducer.FLAG_NONE);
  Expect.isTrue(producer.status.isOk);
  Expect.isNotNull(twoPhaseWrite);
  Expect.isTrue(twoPhaseWrite.lengthInBytes >= 20);

  String world = "world";
  twoPhaseWrite.buffer.asUint8List().setAll(0, world.codeUnits);
  producer.endWrite(Uint8List.BYTES_PER_ELEMENT * world.codeUnits.length);
  Expect.isTrue(producer.status.isOk);

  // Read one character from consumer.
  int read = consumer.read(buffer, 1, MojoDataPipeConsumer.FLAG_NONE);
  Expect.isTrue(consumer.status.isOk);
  Expect.equals(read, 1);

  // Close the producer.
  result = producer.handle.close();
  Expect.isTrue(result.isOk);

  // Consumer should still be readable.
  result = consumer.handle.wait(MojoHandleSignals.READABLE, 0);
  Expect.isTrue(result.isOk);

  // Get the number of remaining bytes.
  int remaining = consumer.read(
      null, 0, MojoDataPipeConsumer.FLAG_QUERY);
  Expect.isTrue(consumer.status.isOk);
  Expect.equals(remaining, "hello world".length - 1);

  // Do a two-phase read.
  ByteData twoPhaseRead = consumer.beginRead(
      remaining, MojoDataPipeConsumer.FLAG_NONE);
  Expect.isTrue(consumer.status.isOk);
  Expect.isNotNull(twoPhaseRead);
  Expect.isTrue(twoPhaseRead.lengthInBytes <= remaining);

  Uint8List uint8_list = buffer.buffer.asUint8List();
  uint8_list.setAll(1, twoPhaseRead.buffer.asUint8List());
  uint8_list = uint8_list.sublist(0, 1 + twoPhaseRead.lengthInBytes);

  consumer.endRead(twoPhaseRead.lengthInBytes);
  Expect.isTrue(consumer.status.isOk);

  String helloWorld = new String.fromCharCodes(
      uint8_list.toList());
  Expect.equals("hello world", helloWorld);

  result = consumer.handle.close();
  Expect.isTrue(result.isOk);
}


basicSharedBufferTest() {
  MojoSharedBuffer mojoBuffer = new MojoSharedBuffer(
      100, MojoSharedBuffer.CREATE_FLAG_NONE);
  Expect.isNotNull(mojoBuffer);
  Expect.isNotNull(mojoBuffer.status);
  Expect.isTrue(mojoBuffer.status.isOk);
  Expect.isNotNull(mojoBuffer.handle);
  Expect.isTrue(mojoBuffer.handle is RawMojoHandle);
  Expect.notEquals(mojoBuffer.handle.h, RawMojoHandle.INVALID);

  mojoBuffer.map(0, 100, MojoSharedBuffer.MAP_FLAG_NONE);
  Expect.isNotNull(mojoBuffer.status);
  Expect.isTrue(mojoBuffer.status.isOk);
  Expect.isNotNull(mojoBuffer.mapping);
  Expect.isTrue(mojoBuffer.mapping is ByteData);

  mojoBuffer.mapping.setInt8(50, 42);

  MojoSharedBuffer duplicate = new MojoSharedBuffer.duplicate(
      mojoBuffer, MojoSharedBuffer.DUPLICATE_FLAG_NONE);
  Expect.isNotNull(duplicate);
  Expect.isNotNull(duplicate.status);
  Expect.isTrue(duplicate.status.isOk);
  Expect.isTrue(duplicate.handle is RawMojoHandle);
  Expect.notEquals(duplicate.handle.h, RawMojoHandle.INVALID);
  Expect.isNotNull(duplicate.mapping);
  Expect.isTrue(duplicate.mapping is ByteData);

  mojoBuffer.close();
  mojoBuffer = null;

  duplicate.mapping.setInt8(51, 43);

  duplicate.unmap();
  Expect.isNotNull(duplicate.status);
  Expect.isTrue(duplicate.status.isOk);
  Expect.isNull(duplicate.mapping);

  duplicate.map(50, 50, MojoSharedBuffer.MAP_FLAG_NONE);
  Expect.isNotNull(duplicate.status);
  Expect.isTrue(duplicate.status.isOk);
  Expect.isNotNull(duplicate.mapping);
  Expect.isTrue(duplicate.mapping is ByteData);

  Expect.equals(duplicate.mapping.getInt8(0), 42);
  Expect.equals(duplicate.mapping.getInt8(1), 43);

  duplicate.unmap();
  Expect.isNotNull(duplicate.status);
  Expect.isTrue(duplicate.status.isOk);
  Expect.isNull(duplicate.mapping);

  duplicate.close();
  duplicate = null;
}


main() async {
  await mojoInit();
  invalidHandleTest();
  basicMessagePipeTest();
  basicDataPipeTest();
  basicSharedBufferTest();
  mojoShutdown();
}
