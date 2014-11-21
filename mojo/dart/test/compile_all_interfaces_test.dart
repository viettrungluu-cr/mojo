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

// Make sure that we are generating valid Dart code for all mojom interface
// tests.
// vmoptions: --compile_all

import 'package:mojo/public/interfaces/application/application.mojom.dart';
import 'package:mojo/public/interfaces/application/service_provider.mojom.dart';
import 'package:mojo/public/interfaces/application/shell.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/math_calculator.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/no_module.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/rect.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/regression_tests.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/sample_factory.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/sample_import2.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/sample_import.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/sample_interfaces.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/sample_service.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/serialization_test_structs.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/test_structs.mojom.dart';
import 'package:mojo/public/interfaces/bindings/tests/validation_test_interfaces.mojom.dart';

int main() {
  return 0;
}
