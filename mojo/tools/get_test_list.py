#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Central list of tests to run (as appropriate for a given config). Add tests
to run by modifying this file."""


import argparse
import json
import os
import sys

from mopy.config import Config
from mopy.paths import Paths


def _TestTypesMatch(types_to_run, this_tests_types):
  """types_to_run should be a set of test types to run. this_test_types should
  be an iterable. Returns true if any type in the latter is also in the former,
  i.e., if the two are not disjoint."""
  return not types_to_run.isdisjoint(this_tests_types)


def _MakeEntry(name, command):
  """Makes an entry with the given name to run the given command."""
  return {"name": name, "command": command}


def _MakeMaybeXvfbEntry(config, name, command):
  """Makes an entry with the given name to run the given command, using xvfb if
  appropriate."""
  real_command = ["python"]
  if config.target_os == Config.OS_LINUX:
    paths = Paths(config)
    real_command += ["./testing/xvfb.py", paths.SrcRelPath(paths.build_dir)]
  real_command += command
  return _MakeEntry(name, real_command)


def GetTestList(config):
  """Gets the list of tests to run for the given config. The test list (which is
  returned) is just a list of dictionaries, each dictionary having two required
  fields:
    {
      "name": "Short name",
      "command": ["python", "test_runner.py", "--some", "args"]
    }
  """

  types_to_run = set(config.test_types)

  # See above for a description of the test list.
  test_list = []

  # TODO(vtl): Currently, we only know how to run tests on Linux or Windows.
  # (The "real" problem is that we only know how to run tests when the target OS
  # is the same as the host OS.)
  if config.target_os not in (Config.OS_LINUX, Config.OS_WINDOWS):
    return test_list

  paths = Paths(config)
  build_dir = paths.SrcRelPath(paths.build_dir)

  # Tests run by default -------------------------------------------------------

  # C++ unit tests:
  if _TestTypesMatch(types_to_run, [Config.TEST_TYPE_DEFAULT,
                                    Config.TEST_TYPE_UNIT]):
    test_list.append(_MakeMaybeXvfbEntry(
        config,
        "Unit tests",
        [os.path.join("mojo", "tools", "test_runner.py"),
         os.path.join("mojo", "tools", "data", "unittests"),
         build_dir,
         "mojob_test_successes"]))

  # C++ app tests:
  if _TestTypesMatch(types_to_run, [Config.TEST_TYPE_DEFAULT, "app"]):
    test_list.append(_MakeMaybeXvfbEntry(
        config,
        "App tests",
        [os.path.join("mojo", "tools", "apptest_runner.py"),
         os.path.join("mojo", "tools", "data", "apptests"),
         build_dir]))

  # Python unit tests:
  if _TestTypesMatch(types_to_run, [Config.TEST_TYPE_DEFAULT,
                                    Config.TEST_TYPE_UNIT]):
    test_list.append(_MakeEntry(
        "Python unit tests",
        ["python", os.path.join("mojo", "tools", "run_mojo_python_tests.py")]))

  # Python bindings tests (Linux-only):
  if (config.target_os == Config.OS_LINUX and
      _TestTypesMatch(types_to_run, [Config.TEST_TYPE_DEFAULT,
                                     Config.TEST_TYPE_UNIT])):
    test_list.append(_MakeEntry(
        "Python bindings tests",
        ["python",
         os.path.join("mojo", "tools", "run_mojo_python_bindings_tests.py"),
         "--build-dir=" + build_dir]))

  # Sky tests (Linux-only):
  if (config.target_os == Config.OS_LINUX and
      _TestTypesMatch(types_to_run, [Config.TEST_TYPE_DEFAULT, "sky"])):
    sky_command = [
        "sky/tools/test_sky",
        "-t", "Debug" if config.is_debug else "Release",
        "--no-new-test-results",
        "--no-show-results",
        "--verbose"]
    if "builder_name" in config.values:
      sky_command += ["--builder-name", config.values["builder_name"]]
    if "build_number" in config.values:
      sky_command += ["--build-number", config.values["build_number"]]
    if "master_name" in config.values:
      sky_command += ["--master-name", config.values["master_name"]]
    if "test_results_server" in config.values:
      sky_command += ["--test-results-server",
                      config.values["test_results_server"]]
    test_list.append(_MakeMaybeXvfbEntry(config, "Sky tests", sky_command))

  # Perf tests -----------------------------------------------------------------

  if (config.target_os == Config.OS_LINUX and
      _TestTypesMatch(types_to_run, [Config.TEST_TYPE_PERF])):
    test_list.append(_MakeEntry(
        "Perf tests",
        [os.path.join(build_dir, 'mojo_public_system_perftests')]))

  # Other (non-default) tests --------------------------------------------------

  # TODO(vtl): Dart tests ...

  return test_list


def main():
  parser = argparse.ArgumentParser(description="Gets tests to execute.")
  parser.add_argument("config_file", metavar="config.json",
                      type=argparse.FileType("rb"),
                      help="Input JSON file with test configuration.")
  parser.add_argument("test_list_file", metavar="test_list.json", nargs="?",
                      type=argparse.FileType("wb"), default=sys.stdout,
                      help="Output JSON file with test list.")
  args = parser.parse_args()

  config = Config(**json.load(args.config_file))
  test_list = GetTestList(config)
  json.dump(test_list, args.test_list_file, indent=2)
  args.test_list_file.write("\n")

  return 0


if __name__ == "__main__":
  sys.exit(main())
