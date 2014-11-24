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


def print_output(command_line, output):
  print command_line
  print 72 * '-'
  print output
  print 72 * '-'


def try_command_line(command_line):
  """Returns the output of a command line or an empty string on error."""
  _logging.debug("Running command line: %s" % command_line)
  try:
    output = subprocess.check_output(command_line, stderr=subprocess.STDOUT)
    return output
  except subprocess.CalledProcessError as e:
    print "Failed with exit code %d, command line, and output:" % e.returncode
    print_output(command_line, e.output)
  except Exception as e:
    print "Failed with command line and exception:"
    print_output(command_line, e)
  return None


def get_fixtures(command_line):
  """Returns the "Test.Fixture" list from a --gtest_list_tests commandline."""
  list_output = try_command_line(command_line)
  _logging.debug("Tests listed:\n%s" % list_output)
  try:
    return gtest_list_tests(list_output)
  except Exception as e:
    print "Failed to get test fixtures with command line and exception:"
    print_output(command_line, e)
    return []


def run_test(command_line):
  """Runs a command line and checks the output for signs of gtest failure."""
  output = try_command_line(command_line)
  # Fail on output with gtest's "[  FAILED  ]" or a lack of "[  PASSED  ]".
  # The latter condition ensures failure on broken command lines or output.
  # Check output instead of exit codes because mojo_shell always exits with 0.
  if output.find("[  FAILED  ]") != -1 or output.find("[  PASSED  ]") == -1:
    print "Failed test with command line and output:"
    print_output(command_line, output)
    return False
  _logging.debug("Succeeded with output:\n%s" % output)
  return True


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

  exit_code = 0
  for apptest in apptest_list:
    print "Running " + apptest + "...",
    sys.stdout.flush()

    # List the apptest fixtures so they can be run independently for isolation.
    # TODO(msw): Run some apptests without fixture isolation?
    command = [mojo_shell,
               "--args-for={0} --gtest_list_tests".format(apptest),
               apptest]
    fixtures = get_fixtures(command)

    if not fixtures:
      print "Failed with no tests found."
      exit_code = 1
      continue

    apptest_result = "Succeeded"
    for fixture in fixtures:
      # ExampleApplicationTest.CheckCommandLineArg checks --example_apptest_arg.
      # TODO(msw): Enable passing arguments to tests down from the test harness.
      command = [mojo_shell,
          "--args-for={0} --example_apptest_arg --gtest_filter={1}".format(
		      apptest, fixture),
          "--args-for=mojo:native_viewport_service --use-headless-config --use-osmesa",
          apptest]
      if not run_test(command):
        apptest_result = "Failed test(s) in " + apptest
        exit_code = 1
    print apptest_result

  return exit_code

if __name__ == '__main__':
  sys.exit(main(sys.argv))
