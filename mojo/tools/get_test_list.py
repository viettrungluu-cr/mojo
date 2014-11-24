#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Central list of tests to run (as appropriate for a given test config). Add
tests to run by modifying this file."""


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
  return _MakeEntry(name, real_command)


def GetTestList(test_config):
  """Gets the list of tests to run for the given test config. The test list
  (which is returned) is just a list of dictionaries, each dictionary having two
  required fields:
    {
      "name": "Short name",
      "command": ["python", "test_runner.py", "--some", "args"]
    }
  """

  types_to_run = set(test_config["test_types"])

  # See above for a description of the test list.
  test_list = []

  # TODO(vtl): Currently, we only know how to run tests on Linux or Windows.
  # (The "real" problem is that we only know how to run tests when the target OS
  # is the same as the host OS.)
  if (test_config["target_os"] != mopy.test_config.OS_LINUX and
      test_config["target_os"] != mopy.test_config.OS_WINDOWS):
    return test_list

  build_dir = mopy.test_config.GetBuildDir(test_config)

  # Tests run by default -------------------------------------------------------

  # C++ unit tests:
  if _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT,
                                    mopy.test_config.TEST_TYPE_UNIT]):
    test_list.append(_MakeMaybeXvfbEntry(
        test_config,
        "Unit tests",
        [os.path.join("mojo", "tools", "test_runner.py"),
         os.path.join("mojo", "tools", "data", "unittests"),
         build_dir,
         "mojob_test_successes"]))

  # C++ app tests:
  if _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT, "app"]):
    test_list.append(_MakeMaybeXvfbEntry(
        test_config,
        "App tests",
        [os.path.join("mojo", "tools", "apptest_runner.py"),
         os.path.join("mojo", "tools", "data", "apptests"),
         build_dir]))

  # Python unit tests:
  if _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT,
                                    mopy.test_config.TEST_TYPE_UNIT]):
    test_list.append(_MakeEntry(
        "Python unit tests",
        ["python", os.path.join("mojo", "tools", "run_mojo_python_tests.py")]))

  # Python bindings tests (Linux-only):
  if (test_config["target_os"] == mopy.test_config.OS_LINUX and
      _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT,
                                     mopy.test_config.TEST_TYPE_UNIT])):
    test_list.append(_MakeEntry(
        "Python bindings tests",
        ["python",
         os.path.join("mojo", "tools", "run_mojo_python_bindings_tests.py"),
         "--build-dir=" + build_dir]))

  # Sky tests (Linux-only):
  if (test_config["target_os"] == mopy.test_config.OS_LINUX and
      _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_DEFAULT,
                                     "sky"])):
    sky_command = [
        "sky/tools/test_sky",
        "-t", "Debug" if test_config["build_type"] ==
                         mopy.test_config.BUILD_TYPE_DEBUG else "Release",
        "--no-new-test-results",
        "--no-show-results",
        "--verbose"]
    if "builder_name" in test_config:
      sky_command += ["--builder-name", test_config["builder_name"]]
    if "build_number" in test_config:
      sky_command += ["--build-number", test_config["build_number"]]
    if "master_name" in test_config:
      sky_command += ["--master-name", test_config["master_name"]]
    if "test_results_server" in test_config:
      sky_command += ["--test-results-server",
                      test_config["test_results_server"]]
    test_list.append(_MakeMaybeXvfbEntry(test_config, "Sky tests", sky_command))

  # Perf tests -----------------------------------------------------------------

  if (test_config["target_os"] == mopy.test_config.OS_LINUX and
      _TestTypesMatch(types_to_run, [mopy.test_config.TEST_TYPE_PERF])):
    test_list.append(_MakeEntry(
        "Perf tests",
        [os.path.join(build_dir, 'mojo_public_system_perftests')]))

  # Other (non-default) tests --------------------------------------------------

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
