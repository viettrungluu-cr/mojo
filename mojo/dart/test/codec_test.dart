// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:mojo/dart/testing/expect.dart';
import 'package:mojo/public/dart/bindings.dart' as bindings;
import 'package:mojo/public/dart/core.dart' as core;

class TestBar implements bindings.MojoType<TestBar> {
  static const int TYPE_VERTICAL = 1;
  static const int TYPE_HORIZONTAL = TYPE_VERTICAL + 1;
  static const int TYPE_BOTH = TYPE_VERTICAL + 1;
  static const int TYPE_INVALID = TYPE_BOTH + 1;

  int alpha = 0xff;
  int beta = 0;
  int gamma = 0;
  int type = TYPE_VERTICAL;

  TestBar();

  static const int encodedSize = bindings.kStructHeaderSize + 8;

  static TestBar decode(bindings.MojoDecoder decoder) {
    var val = new TestBar();
    var num_bytes = decoder.readUint32();
    var num_fields = decoder.readUint32();
    val.alpha = decoder.decodeStruct(bindings.Uint8);
    val.beta = decoder.decodeStruct(bindings.Uint8);
    val.gamma = decoder.decodeStruct(bindings.Uint8);
    decoder.skip(1);
    val.type = decoder.decodeStruct(bindings.Int32);
    return val;
  }

  static void encode(bindings.MojoEncoder encoder, TestBar val) {
    encoder.writeUint32(encodedSize);
    encoder.writeUint32(4);
    encoder.encodeStruct(bindings.Uint8, val.alpha);
    encoder.encodeStruct(bindings.Uint8, val.beta);
    encoder.encodeStruct(bindings.Uint8, val.gamma);
    encoder.skip(1);
    encoder.encodeStruct(bindings.Int32, val.type);
  }

  String toString() {
    return "alpha=$alpha, beta=$beta, gamma=$gamma, type=$type";
  }
}


void testBar() {
  var bar = new TestBar();
  bar.alpha = 1;
  bar.beta = 2;
  bar.gamma = 3;
  bar.type = 0x08070605;

  int name = 42;
  int payloadSize = TestBar.encodedSize;

  var builder = new bindings.MessageBuilder(name, payloadSize);
  builder.encodeStruct(TestBar, bar);
  var message = builder.finish();

  var expectedMemory = new Uint8List.fromList([
    16, 0, 0, 0,
     2, 0, 0, 0,
    42, 0, 0, 0,
     0, 0, 0, 0,

    16, 0, 0, 0,
     4, 0, 0, 0,

     1, 2, 3, 0,
     5, 6, 7, 8,
  ]);

  var actualMemory = message.buffer.buffer.asUint8List();
  Expect.listEquals(expectedMemory, actualMemory);

  var reader = new bindings.MessageReader(message);

  Expect.equals(payloadSize, reader.payloadSize);
  Expect.equals(name, reader.name);

  var bar2 = reader.decodeStruct(TestBar);

  Expect.equals(bar.alpha, bar2.alpha);
  Expect.equals(bar.beta, bar2.beta);
  Expect.equals(bar.gamma, bar2.gamma);
  Expect.equals(bar.type, bar2.type);
}


class TestFoo implements bindings.MojoType<TestFoo> {
  static const String kFooby = "Fooby";

  int x = 0;
  int y = 0;
  bool a = true;
  bool b = false;
  bool c = false;
  core.RawMojoHandle source = null;
  TestBar bar = null;
  List<int> data = null;
  List<TestBar> extra_bars = null;
  String name = kFooby;
  List<core.RawMojoHandle> input_streams = null;
  List<core.RawMojoHandle> output_streams = null;
  List<List<bool>> array_of_array_of_bools = null;
  List<List<List<String>>> multi_array_of_strings = null;
  List<bool> array_of_bools = null;

  TestFoo();

  static const int encodedSize = bindings.kStructHeaderSize + 88;

  static TestFoo decode(bindings.MojoDecoder decoder) {
    int packed;
    var val = new TestFoo();
    var num_bytes = decoder.readUint32();
    var num_fields = decoder.readUint32();
    val.x = decoder.decodeStruct(bindings.Int32);
    val.y = decoder.decodeStruct(bindings.Int32);
    packed = decoder.readUint8();
    val.a = ((packed >> 0) & 1) != 0 ? true : false;
    val.b = ((packed >> 1) & 1) != 0 ? true : false;
    val.c = ((packed >> 2) & 1) != 0 ? true : false;
    decoder.skip(1);
    decoder.skip(1);
    decoder.skip(1);
    val.source = decoder.decodeStruct(bindings.NullableHandle);
    val.bar = decoder.decodeStructPointer(TestBar);
    val.data = decoder.decodeArrayPointer(bindings.Uint8);
    val.extra_bars =
        decoder.decodeArrayPointer(new bindings.PointerTo(TestBar));
    val.name = decoder.decodeStruct(bindings.MojoString);
    val.input_streams = decoder.decodeArrayPointer(bindings.Handle);
    val.output_streams = decoder.decodeArrayPointer(bindings.Handle);
    val.array_of_array_of_bools =
        decoder.decodeArrayPointer(new bindings.ArrayOf(bindings.PackedBool));
    val.multi_array_of_strings = decoder.decodeArrayPointer(
        new bindings.ArrayOf(new bindings.ArrayOf(bindings.MojoString)));
    val.array_of_bools = decoder.decodeArrayPointer(bindings.PackedBool);
    return val;
  }

  static void encode(bindings.MojoEncoder encoder, TestFoo val) {
    int packed = 0;
    encoder.writeUint32(encodedSize);
    encoder.writeUint32(15);
    encoder.encodeStruct(bindings.Int32, val.x);
    encoder.encodeStruct(bindings.Int32, val.y);
    packed |= (val.a ? 1 : 0) << 0;
    packed |= (val.b ? 1 : 0) << 1;
    packed |= (val.c ? 1 : 0) << 2;
    encoder.writeUint8(packed);
    encoder.skip(1);
    encoder.skip(1);
    encoder.skip(1);
    encoder.encodeStruct(bindings.NullableHandle, val.source);
    encoder.encodeStructPointer(TestBar, val.bar);
    encoder.encodeArrayPointer(bindings.Uint8, val.data);
    encoder.encodeArrayPointer(new bindings.PointerTo(TestBar), val.extra_bars);
    encoder.encodeStruct(bindings.MojoString, val.name);
    encoder.encodeArrayPointer(bindings.Handle, val.input_streams);
    encoder.encodeArrayPointer(bindings.Handle, val.output_streams);
    encoder.encodeArrayPointer(
        new bindings.ArrayOf(bindings.PackedBool), val.array_of_array_of_bools);
    encoder.encodeArrayPointer(
        new bindings.ArrayOf(new bindings.ArrayOf(bindings.MojoString)),
        val.multi_array_of_strings);
    encoder.encodeArrayPointer(bindings.PackedBool, val.array_of_bools);
  }
}


void testFoo() {
  var foo = new TestFoo();
  foo.x = 0x212B4D5;
  foo.y = 0x16E93;
  foo.a = true;
  foo.b = false;
  foo.c = true;
  foo.bar = new TestBar();
  foo.bar.alpha = 91;
  foo.bar.beta = 82;
  foo.bar.gamma = 73;
  foo.data = [
    4, 5, 6, 7, 8,
  ];
  foo.extra_bars = [
    new TestBar(), new TestBar(), new TestBar(),
  ];
  for (int i = 0; i < foo.extra_bars.length; ++i) {
    foo.extra_bars[i].alpha = 1 * i;
    foo.extra_bars[i].beta = 2 * i;
    foo.extra_bars[i].gamma = 3 * i;
  }
  foo.name = "I am a banana";
  // This is supposed to be a handle, but we fake it with an integer.
  foo.source = new core.RawMojoHandle(23423782);
  foo.array_of_array_of_bools = [
    [true], [false, true]
  ];
  foo.array_of_bools = [
    true, false, true, false, true, false, true, true
  ];


  var name = 31;
  var payloadSize = 304;

  var builder = new bindings.MessageBuilder(name, payloadSize);
  builder.encodeStruct(TestFoo, foo);

  var message = builder.finish();

  var expectedMemory = new Uint8List.fromList([
    /*  0: */   16,    0,    0,    0,    2,    0,    0,    0,
    /*  8: */   31,    0,    0,    0,    0,    0,    0,    0,
    /* 16: */   96,    0,    0,    0,   15,    0,    0,    0,
    /* 24: */ 0xD5, 0xB4, 0x12, 0x02, 0x93, 0x6E, 0x01,    0,
    /* 32: */    5,    0,    0,    0,    0,    0,    0,    0,
    /* 40: */   72,    0,    0,    0,    0,    0,    0,    0,
  ]);
  // TODO(abarth): Test more of the message's raw memory.
  var allActualMemory = message.buffer.buffer.asUint8List();
  var actualMemory = allActualMemory.sublist(0, expectedMemory.length);
  Expect.listEquals(expectedMemory, actualMemory);

  var expectedHandles = <core.RawMojoHandle>[
    new core.RawMojoHandle(23423782),
  ];

  Expect.listEquals(expectedHandles, message.handles);

  var reader = new bindings.MessageReader(message);

  Expect.equals(payloadSize, reader.payloadSize);
  Expect.equals(name, reader.name);

  var foo2 = reader.decodeStruct(TestFoo);

  Expect.equals(foo.x, foo2.x);
  Expect.equals(foo.y, foo2.y);

  Expect.equals(foo.a, foo2.a);
  Expect.equals(foo.b, foo2.b);
  Expect.equals(foo.c, foo2.c);

  Expect.equals(foo.bar.alpha, foo2.bar.alpha);
  Expect.equals(foo.bar.beta, foo2.bar.beta);
  Expect.equals(foo.bar.gamma, foo2.bar.gamma);
  Expect.equals(foo.bar.type, foo2.bar.type);
  Expect.listEquals(foo.data, foo2.data);

  for (int i = 0; i < foo2.extra_bars.length; i++) {
    Expect.equals(foo.extra_bars[i].alpha, foo2.extra_bars[i].alpha);
    Expect.equals(foo.extra_bars[i].beta, foo2.extra_bars[i].beta);
    Expect.equals(foo.extra_bars[i].gamma, foo2.extra_bars[i].gamma);
    Expect.equals(foo.extra_bars[i].type, foo2.extra_bars[i].type);
  }

  Expect.equals(foo.name, foo2.name);
  Expect.equals(foo.source, foo2.source);

  Expect.listEquals(foo.array_of_bools, foo2.array_of_bools);
  for (int i = 0; i < foo2.array_of_array_of_bools.length; i++) {
    Expect.listEquals(foo.array_of_array_of_bools[i],
                      foo2.array_of_array_of_bools[i]);
  }
}


class TestRect implements bindings.MojoType<TestRect> {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;

  TestRect();

  static const int encodedSize = bindings.kStructHeaderSize + 16;

  static TestRect decode(bindings.MojoDecoder decoder) {
    var val = new TestRect();
    var num_bytes = decoder.readUint32();
    var num_fields = decoder.readUint32();
    val.x = decoder.decodeStruct(bindings.Int32);
    val.y = decoder.decodeStruct(bindings.Int32);
    val.width = decoder.decodeStruct(bindings.Int32);
    val.height = decoder.decodeStruct(bindings.Int32);
    return val;
  }

  static void encode(bindings.MojoEncoder encoder, TestRect val) {
    encoder.writeUint32(encodedSize);
    encoder.writeUint32(4);
    encoder.encodeStruct(bindings.Int32, val.x);
    encoder.encodeStruct(bindings.Int32, val.y);
    encoder.encodeStruct(bindings.Int32, val.width);
    encoder.encodeStruct(bindings.Int32, val.height);
  }

  bool operator ==(TestRect other) {
    return (x == other.x) && (y == other.y) && (width == other.width) &&
           (height == other.height);
  }
}


TestRect createRect(int x, int y, int width, int height) {
  var r = new TestRect();
  r.x = x;
  r.y = y;
  r.width = width;
  r.height = height;
  return r;
}


class TestNamedRegion implements bindings.MojoType<TestNamedRegion> {
  String name = null;
  List<TestRect> rects = null;

  TestNamedRegion();

  static const int encodedSize = bindings.kStructHeaderSize + 16;

  static TestNamedRegion decode(bindings.MojoDecoder decoder) {
    var val = new TestNamedRegion();
    var num_bytes = decoder.readUint32();
    var num_fields = decoder.readUint32();
    val.name = decoder.decodeStruct(bindings.NullableMojoString);
    val.rects = decoder.decodeArrayPointer(new bindings.PointerTo(TestRect));
    return val;
  }

  static void encode(bindings.MojoEncoder encoder, TestNamedRegion val) {
    encoder.writeUint32(TestNamedRegion.encodedSize);
    encoder.writeUint32(2);
    encoder.encodeStruct(bindings.NullableMojoString, val.name);
    encoder.encodeArrayPointer(new bindings.PointerTo(TestRect), val.rects);
  }
}


testNamedRegion() {
  var r = new TestNamedRegion();
  r.name = "rectangle";
  r.rects = [createRect(1, 2, 3, 4), createRect(10, 20, 30, 40)];

  var builder = new bindings.MessageBuilder(1, TestNamedRegion.encodedSize);
  builder.encodeStruct(TestNamedRegion, r);
  var reader = new bindings.MessageReader(builder.finish());
  var result = reader.decodeStruct(TestNamedRegion);

  Expect.equals("rectangle", result.name);
  Expect.equals(createRect(1, 2, 3, 4), result.rects[0]);
  Expect.equals(createRect(10, 20, 30, 40), result.rects[1]);
}


void testAlign() {
  List aligned = [
    0, // 0
    8, // 1
    8, // 2
    8, // 3
    8, // 4
    8, // 5
    8, // 6
    8, // 7
    8, // 8
    16, // 9
    16, // 10
    16, // 11
    16, // 12
    16, // 13
    16, // 14
    16, // 15
    16, // 16
    24, // 17
    24, // 18
    24, // 19
    24, // 20
  ];
  for (int i = 0; i < aligned.length; ++i) {
    Expect.equals(bindings.align(i), aligned[i]);
  }
}

void encodeDecode(Object t, Object input, Object expected, [int encoded_size]) {
  int name = 42;
  int payloadSize =
      (encoded_size != null) ? encoded_size : bindings.getEncodedSize(t);

  var builder = new bindings.MessageBuilder(name, payloadSize);
  builder.encodeStruct(t, input);
  var message = builder.finish();

  var reader = new bindings.MessageReader(message);
  Expect.equals(payloadSize, reader.payloadSize);
  Expect.equals(name, reader.name);

  var result = reader.decodeStruct(t);
  Expect.equals(expected, result);
}


void testTypes() {
  encodeDecode(bindings.MojoString, "banana", "banana", 24);
  encodeDecode(bindings.NullableMojoString, null, null, 8);
  encodeDecode(bindings.Int8, -1, -1);
  encodeDecode(bindings.Int8, 0xff, -1);
  encodeDecode(bindings.Int16, -1, -1);
  encodeDecode(bindings.Int16, 0xff, 0xff);
  encodeDecode(bindings.Int16, 0xffff, -1);
  encodeDecode(bindings.Int32, -1, -1);
  encodeDecode(bindings.Int32, 0xffff, 0xffff);
  encodeDecode(bindings.Int32, 0xffffffff, -1);
  encodeDecode(bindings.Float, 1.0, 1.0);
  encodeDecode(bindings.Double, 1.0, 1.0);
}


testUtf8() {
  var str = "B\u03ba\u1f79";  // some UCS-2 codepoints
  var name = 42;
  var payloadSize = 24;

  var builder = new bindings.MessageBuilder(name, payloadSize);
  var encoder = builder.createEncoder(8);
  encoder.encodeStringPointer(str);
  var message = builder.finish();
  var expectedMemory = new Uint8List.fromList([
    /*  0: */   16,    0,    0,    0,    2,    0,    0,    0,
    /*  8: */   42,    0,    0,    0,    0,    0,    0,    0,
    /* 16: */    8,    0,    0,    0,    0,    0,    0,    0,
    /* 24: */   14,    0,    0,    0,    6,    0,    0,    0,
    /* 32: */ 0x42, 0xCE, 0xBA, 0xE1, 0xBD, 0xB9,    0,    0,
  ]);
  var actualMemory = message.buffer.buffer.asUint8List();
  Expect.equals(expectedMemory.length, actualMemory.length);
  Expect.listEquals(expectedMemory, actualMemory);

  var reader = new bindings.MessageReader(message);
  Expect.equals(payloadSize, reader.payloadSize);
  Expect.equals(name, reader.name);

  var str2 = reader.decoder.decodeStringPointer();
  Expect.equals(str, str2);
}


main() {
  testAlign();
  testTypes();
  testBar();
  testFoo();
  testNamedRegion();
  testUtf8();
}