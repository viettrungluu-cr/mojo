#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import subprocess
import sys
import tempfile
import time
import zipfile

from mopy.paths import Paths
from mopy.version import Version

paths = Paths(os.path.join("out", "Release"))

sys.path.insert(0, os.path.join(paths.src_root, "tools"))
# pylint: disable=F0401
import find_depot_tools

depot_tools_path = find_depot_tools.add_depot_tools_to_path()
gsutil_exe = os.path.join(depot_tools_path, "third_party", "gsutil", "gsutil")

def upload(dry_run, verbose):
  dest = "gs://mojo/shell/" + Version().version + "/linux-x64.zip"

  with tempfile.NamedTemporaryFile() as zip_file:
    with zipfile.ZipFile(zip_file, 'w') as z:
      with open(paths.mojo_shell_path) as shell_binary:
        zipinfo = zipfile.ZipInfo("mojo_shell")
        zipinfo.external_attr = 0777 << 16L
        zipinfo.compress_type = zipfile.ZIP_DEFLATED
        zipinfo.date_time = time.gmtime(os.path.getmtime(paths.mojo_shell_path))
        if verbose:
          print "zipping %s" % paths.mojo_shell_path
        z.writestr(zipinfo, shell_binary.read())
    if dry_run:
      print str([gsutil_exe, "cp", zip_file.name, dest])
    else:
      subprocess.check_call([gsutil_exe, "cp", zip_file.name, dest])

def main():
  parser = argparse.ArgumentParser(description="Upload mojo_shell binary to "+
      "google storage")
  parser.add_argument("-n", "--dry_run", help="Dry run, do not actually "+
      "upload", action="store_true")
  parser.add_argument("-v", "--verbose", help="Verbose mode",
      action="store_true")
  args = parser.parse_args()
  upload(args.dry_run, args.verbose)
  return 0

if __name__ == "__main__":
  sys.exit(main())
