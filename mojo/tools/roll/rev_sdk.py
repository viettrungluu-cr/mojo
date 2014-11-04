#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
from utils import commit
from utils import mojo_root_dir
from utils import system

dirs_to_clone = [
  "mojo/edk",
  "mojo/public",
  "mojo/services/public",
]

def rev(source_dir, chromium_dir):
  src_commit = system(["git", "show-ref", "HEAD", "-s"], cwd=source_dir).strip()

  for d in dirs_to_clone:
    print "removing directory %s" % d
    system(["git", "rm", "-r", d], cwd=chromium_dir)
    print "cloning directory %s" % d
    files = system(["git", "ls-files", d], cwd=source_dir)
    for f in files.splitlines():
      dest_path = os.path.join(chromium_dir, f)
      system(["mkdir", "-p", os.path.dirname(dest_path)])
      system(["cp", os.path.join(source_dir, f), dest_path])
    os.chdir(chromium_dir)
    system(["git", "add", d], cwd=chromium_dir)

  with open("mojo/public/VERSION", "w") as version_file:
    version_file.write(src_commit)
  system(["git", "add", "mojo/public/VERSION"], cwd=chromium_dir)
  commit("Update mojo sdk to rev " + src_commit, cwd=chromium_dir)

if len(sys.argv) != 2:
  print "usage: rev_sdk.py <chromium source dir>"
  sys.exit(1)

rev(mojo_root_dir, sys.argv[1])
