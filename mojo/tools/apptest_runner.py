#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A test runner for gtest application tests."""

import logging
import os
import platform
import subprocess
import sys

_logging = logging.getLogger()

from mopy.gtest_list_tests import gtest_list_tests

def main(argv):
  logging.basicConfig()
  # Uncomment to debug:
  # _logging.setLevel(logging.DEBUG)

  if len(argv) != 3:
    print "Usage: %s gtest_app_list_file root_dir" % os.path.basename(argv[0])
    return 0

  _logging.debug("Test list file: %s", argv[1])
  with open(argv[1], 'rb') as f:
    apptest_list = [y for y in [x.strip() for x in f.readlines()] \
                        if y and y[0] != '#']
  _logging.debug("Test list: %s" % apptest_list)

  # Run gtests with color if we're on a TTY (and we're not being told explicitly
  # what to do).
  if sys.stdout.isatty() and 'GTEST_COLOR' not in os.environ:
    _logging.debug("Setting GTEST_COLOR=yes")
    os.environ['GTEST_COLOR'] = 'yes'

  mojo_shell = "./" + argv[2] + "/mojo_shell"
  if platform.system() == 'Windows':
    mojo_shell += ".exe"

  for apptest in apptest_list:
    print "Running " + apptest + "...",
    sys.stdout.flush()

    # List the apptest fixtures so they can be run independently for isolation.
    # TODO(msw): Run some apptests without fixture isolation?
    list_command_line = [mojo_shell, apptest + " --gtest_list_tests"]
    _logging.debug("Listing tests: %s" % list_command_line)
    try:
      # TODO(msw): Need to fail on errors! mojo_shell always exits with 0!
      list_output = subprocess.check_output(list_command_line)
      _logging.debug("Tests listed:\n%s" % list_output)
    except subprocess.CalledProcessError as e:
      print "Failed with exit code %d and output:" % e.returncode
      print 72 * '-'
      print e.output
      print 72 * '-'
      return 1
    except OSError as e:
      print "  Failed to start test"
      return 1

    test_list = gtest_list_tests(list_output)
    for test in test_list:
      # TODO(msw): enable passing arguments to tests down from the test harness.
      command_line = [
          mojo_shell,
          "--args-for={0} --example_apptest_arg --gtest_filter={1}".format(
              apptest, test),
          apptest]

      _logging.debug("Running %s..." % command_line)
      try:
        # TODO(msw): Need to fail on errors! mojo_shell always exits with 0!
        subprocess.check_output(command_line, stderr=subprocess.STDOUT)
        _logging.debug("Succeeded")
      except subprocess.CalledProcessError as e:
        print "Failed with exit code %d and output:" % e.returncode
        print 72 * '-'
        print e.output
        print 72 * '-'
        return 1
      except OSError as e:
        print "  Failed to start test"
        return 1
    print "Succeeded"

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
