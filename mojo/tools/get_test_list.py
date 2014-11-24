#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#TODO
"""TODO"""


import argparse
import json
import os
import sys

import mopy.test_config


def _TestTypesMatch(types_to_run, this_tests_types):
  """types_to_run should be a set of test types to run. this_test_types should
  be an iterable. Returns true if any type in the latter is also in the former,
  i.e., if the two are not disjoint."""
  return not types_to_run.isdisjoint(this_tests_types)


def _MakeEntry(name, command):
  """Makes an entry with the given name to run the given command."""
  return {"name": name, "command": command}


def _MakeMaybeXvfbEntry(test_config, name, command):
  """Makes an entry with the given name to run the given command, using xvfb if
  appropriate."""
  real_command = ["python"]
  if test_config["target_os"] == mopy.test_config.OS_LINUX:
    real_command += ["./testing/xvfb.py",
                     mopy.test_config.GetBuildDir(test_config)]
  real_command += command
  return _MakeEntry(name, command)


def GetTestList(test_config):
  types_to_run = set(test_config["test_types"])

  # The test list is just a list of dictionaries, each dictionary having two
  # required fields:
  #   {
  #     "name": "Short name",
  #     "command": ["python", "test_runner.py", "--some", "args"]
  #   }

  test_list = []
  build_dir = mopy.test_config.GetBuildDir(test_config)

  # C++ unit tests:
  if _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT,
                                    mopy.test_config.TEST_TYPE_UNIT]):
    test_list.append(_MakeMaybeXvfbEntry(
        [os.path.join("mojo", "tools", "test_runner.py"),
         os.path.join("mojo", "tools", "data", "unittests"),
         build_dir,
         "mojob_test_successes"]))

  # C++ app tests:
  if _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT]):
    test_list.append(_MakeMaybeXvfbEntry(
        [os.path.join("mojo", "tools", "apptest_runner.py"),
         os.path.join("mojo", "tools", "data", "apptests"),
         build_dir]))

  # Python unit tests:
  if _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT,
                                    mopy.test_config.TEST_TYPE_UNIT]):
    test_list.append(_MakeEntry(
        ["python", os.path.join("mojo", "tools", "run_mojo_python_tests.py")]))

  # Python bindings tests (Linux-only):
  if (test_config["target_os"] == mopy.test_config.OS_LINUX and
      _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT])):
    test_list.append(_MakeEntry(
        ["python",
         os.path.join("mojo", "tools", "run_mojo_python_bindings_tests.py"),
         "--build-dir=" + build_dir]))

  # TODO(vtl): other tests

  return test_list


def main():
  parser = argparse.ArgumentParser(description="Gets tests to execute.")
  parser.add_argument("config_file", metavar="test_config.json",
                      type=argparse.FileType("rb"),
                      help="Input JSON file with test configuration.")
  parser.add_argument("test_list_file", metavar="test_list.json", nargs="?",
                      type=argparse.FileType("wb"), default=sys.stdout,
                      help="Output JSON file with test list.")
  args = parser.parse_args()

  test_config = json.load(args.config_file)
  test_list = GetTestList(test_config)
  json.dump(test_list, args.test_list_file, indent=2)
  args.test_list_file.write("\n")

  return 0


if __name__ == "__main__":
  sys.exit(main())
