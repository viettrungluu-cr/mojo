# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build/test configurations, which are just dictionaries. This
"defines" the schema and provides some wrappers."""


import json
import os.path
import sys


def _GetHostOS():
  if sys.platform == "linux2":
    return Config.OS_LINUX
  if sys.platform == "darwin":
    return Config.OS_MAC
  if sys.platform == "win32":
    return Config.OS_WINDOWS
  raise NotImplementedError


class Config(object):
  """A Config is basically just a wrapper around a dictionary that species a
  build/test configuration. The dictionary is accessible through the values
  member."""

  # Valid values for target_os:
  OS_ANDROID = "android"
  OS_CHROMEOS = "chromeos"
  OS_LINUX = "linux"
  OS_MAC = "mac"
  OS_WINDOWS = "windows"

  # TODO(vtl): Add clang vs gcc.
  # TODO(vtl): Add ASan/TSan/etc.

  def __init__(self, target_os=None, is_debug=True, **kwargs):
    """Constructs a Config with key-value pairs specified via keyword arguments.
    If target_os is not specified, it will be set to the host OS."""

    assert target_os in (None, Config.OS_ANDROID, Config.OS_CHROMEOS,
                         Config.OS_LINUX, Config.OS_MAC, Config.OS_WINDOWS)
    assert isinstance(is_debug, bool)

    self.values = {}
    self.values["target_os"] = _GetHostOS() if target_os is None else target_os
    self.values["is_debug"] = is_debug

    self.values.update(kwargs)

  # Getters for standard fields ------------------------------------------------

  @property
  def target_os(self):
    """OS of the build/test target."""
    return self.values["target_os"]

  @property
  def is_debug(self):
    """Is Debug build?"""
    return self.values.get("is_debug")
