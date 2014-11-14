# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import timeit


def run(args, paths):
  rounds = 1000

  # Because mojo_benchmark_startup terminates the process immediately when its
  # MojoMain() is called. The overall execution time reflects the startup
  # performance of the mojo shell.
  startup_time = timeit.timeit(
      ("subprocess.call(['%s', 'mojo:mojo_benchmark_startup'])" %
           paths.mojo_shell_path),
      "import subprocess", number=rounds)

  # The execution time of a noop executable is also measured, in order to offset
  # the cost of timeit()/subprocess.call()/etc.
  noop_time = timeit.timeit(
      ("subprocess.call(['%s'])" %
           os.path.join(paths.build_dir, 'mojo_benchmark_startup_noop')),
      "import subprocess", number=rounds)

  # TODO(yzshen): Consider also testing the startup time when
  # mojo_benchmark_startup is served by an HTTP server.

  # Convert the execution time to milliseconds and compute the average for
  # a single run.
  result = (startup_time - noop_time) * 1000 / rounds
  return ("Result: rounds tested: %d; average startup time: %f ms" %
          (rounds, result))
