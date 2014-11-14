# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

class Paths(object):
  """Provides commonly used paths"""

  def __init__(self, build_directory=None):
    """Specify a build_directory to generate paths to binary artifacts"""
    self.src_root = os.path.abspath(os.path.join(__file__,
      os.pardir, os.pardir, os.pardir, os.pardir))
    self.mojo_dir = os.path.join(self.src_root, "mojo")
    if build_directory:
      self.build_dir = os.path.join(self.src_root, build_directory)
      self.mojo_shell_path = os.path.join(self.build_dir, "mojo_shell")
    else:
      self.build_dir = None
      self.mojo_shell_path = None
