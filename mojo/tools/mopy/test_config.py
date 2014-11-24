# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A test configuration is just a dictionary. This "defines" the schema and
provides some utilities."""


import json
import sys


# Valid values for the required fields:
OS_ANDROID = "chromeos"
OS_CHROMEOS = "chromeos"
OS_LINUX = "linux"
OS_MAC = "mac"
OS_WINDOWS = "windows"

ARCH_X64 = "x64"

BUILD_TYPE_DEBUG = "debug"
BUILD_TYPE_RELEASE = "release"

# Test types are arbitrary strings; other test types are also allowed.
TEST_TYPE_DEFAULT = "default"
TEST_TYPE_UNIT = "unit"
TEST_TYPE_PERF = "perf"


def MakeTestConfig(target_os, target_arch, build_type, build_dir, test_types,
                   **kwargs):
  """Makes a test config with the given (required) field values. Optional fields
  may be specified as keyword arguments, and these may override the required
  values."""

  # Required fields:
  test_config = {"target_os": target_os,
                 "target_arch": target_arch,
                 "build_type": build_type,
                 # The build directory, relative to the src/ directory.
                 "build_dir": build_dir,
                 # This is a list of test types to execute. (A test may have
                 # multiple types.)
                 "test_types": test_types}
  test_config.update(kwargs)
  return test_config


def GetDefaultTestConfig(**kwargs):
  """Gets the default test config, guessed from the host configuration and using
  some default values. Extra/overriding field values may be specified as keyword
  arguments."""

  if sys.platform == "darwin":
    target_os = OS_MAC
  elif sys.platform == "linux2":
    target_os = OS_LINUX
  elif sys.platform == "win32":
    target_os = OS_WINDOWS
  else:
    raise NotImplementedError

  # TODO(vtl):
  target_arch = ARCH_X64

  return MakeTestConfig(target_os, target_arch, BUILD_TYPE_DEBUG, "out/Debug",
                        [TEST_TYPE_DEFAULT], **kwargs)


if __name__ == "__main__":
  print json.dumps(GetDefaultTestConfig(), indent=2)
  sys.exit(0)
