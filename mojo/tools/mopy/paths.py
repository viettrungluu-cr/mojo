# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

from .config import Config

class Paths(object):
  """Provides commonly used paths"""

  def __init__(self, config=None):
    """Specify a build_directory to generate paths to binary artifacts"""
    self.src_root = os.path.abspath(os.path.join(__file__,
      os.pardir, os.pardir, os.pardir, os.pardir))
    self.mojo_dir = os.path.join(self.src_root, "mojo")

    if config:
      subdir = ""
      if config.target_os == Config.OS_ANDROID:
        subdir += "android_"
      elif config.target_os == Config.OS_CHROMEOS:
        subdir += "chromeos_"
      subdir += "Debug" if config.is_debug else "Release"
      self.build_dir = os.path.join(self.src_root, "out", subdir)
      self.mojo_shell_path = os.path.join(self.build_dir, "mojo_shell")
    else:
      self.build_dir = None
      self.mojo_shell_path = None

  def RelPath(self, path):
    """Returns the given path, relative to the current directory."""
    return os.path.relpath(path)

  def SrcRelPath(self, path):
    """Returns the given path, relative to self.src_root."""
    return os.path.relpath(path, self.src_root)
