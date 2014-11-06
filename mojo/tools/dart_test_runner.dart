// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This script accepts a relative path as a command line argument and
// looks for files ending in '_test.dart' under that path relative to the
// current working directory. Then, the script runs tests and reports back on
// the results.
//
// Example usage from root source directory:
// $ dart mojo/tools/dart_test_runner.dart out/Debug/gen
//
// The argument path is also set to the Dart package root.

import 'dart:async';
import 'dart:convert';
import 'dart:core';
import 'dart:io';
import 'dart:isolate';


// Where 'path' is a relative path, returns a Future on the list of files
// ending in '_test.dart' under `cwd`/path.
Future<List<String>> findTests(String path) {
  var dir = new Directory(path);
  var tests = [];
  var completer = new Completer();
  dir.list(recursive: true)  // Walk the directory tree.
     .asyncMap((fse) {  // Stat the entries to filter out non-files.
        var completer = new Completer();
        fse.stat().then((info) {
          completer.complete([fse, info]);
        });
        return completer.future;
     })
     .listen((lst) {
        var fse = lst[0];
        var info = lst[1];
        if ((info.type == FileSystemEntityType.FILE) &&
            (fse.path.endsWith('_test.dart'))) {
          tests.add(fse.path);
        }
      }, onDone:() {
        completer.complete(tests);
      });
  return completer.future;
}


// An isolate that runs up to as many tests as there are cores in the system.
void testRunnerIsolate(List args) {
  // The spawner passes this isolate a send port and Dart's package root.
  SendPort sp = args[0];
  String package_root = args[1];

  // Create another send / receive port pair for the spawning isolate to send us
  // messages on.
  var rp = new ReceivePort();
  var rsp = rp.sendPort;

  // Send the spawner the receive port, and request that it begin sending test
  // to run.
  sp.send(rsp);
  sp.send("request");

  int running_tests = 0;
  rp.listen((test) {  // Listen for new tests to run.
    // Tests are run in "checked mode".
    List testargs = ['--checked', '-p', package_root, test];
    Process.start(Platform.executable, testargs).then((process) {
      String stderr = "";

      // Kill the test process after 10 seconds.
      var kill_timer = new Timer(const Duration(seconds:10), process.kill);

      // When the test finishes send back the results. Either "Success" or the
      // contents of stderr.
      process.exitCode.then((code) {
        kill_timer.cancel();
        if (code == 0) {
          sp.send([test, "Success"]);
        } else {
          sp.send([test, stderr]);
        }

        // Room for another test.
        running_tests--;
        if (running_tests < Platform.numberOfProcessors) {
          sp.send("request");
        }
      });

      // Collect the stderr if there is any.
      process.stderr.transform(UTF8.decoder).listen((data) {
        stderr = "$stderr$data";
      });
    });

    running_tests++;
    // If we're not saturated yet, request another test.
    if (running_tests < Platform.numberOfProcessors) {
      sp.send("request");
    }
  });
}


// runTests rate limits the number of tests that are running in parallel by
// spawning an isolate that sends a message whenever the number of tests
// running falls below the number of available cores in the system. Only then
// does runTests send the isolate another test to run.
void runTests(List<String> test_paths, String test_root) {
  // Create a receive port, and prepare a message for the testRunnerIsolate.
  var rp = new ReceivePort();
  List isolate_args = [rp.sendPort, test_root];

  Isolate.spawn(testRunnerIsolate, isolate_args).then((isolate) {
    SendPort sp;
    int test_idx = 0;  // Index of next test to send.
    int finished = 0;  // Number of tests we've heard back about.
    rp.listen((msg) {
      if (msg is SendPort) {
        // First message from spawned isolate is the send port to send it things
        // on.
        sp = msg;
      } else if ((msg is String) && (msg == "request")) {
        // If there is a request for more tests, and we have more, then send
        // the next one.
        if (test_idx < test_paths.length) {
          sp.send(test_paths[test_idx]);
          test_idx++;
        }
      } else if (msg is List) {
        // Print a report of the test results.
        finished++;
        print("${msg[0]}: ${msg[1]}");
        if (finished == test_paths.length) {
          // We got the final result.
          rp.close();  // Closing this port brings down the isolate.
        }
      }
    });
  });
}


main(argv) {
  String root = (argv.isEmpty) ? "." : argv[0];
  findTests(root).then((test_paths) {
    runTests(test_paths, root);
  });
}
