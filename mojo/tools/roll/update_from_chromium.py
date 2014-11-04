#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import os
import subprocess
import sys
import urllib2
from utils import commit
from utils import mojo_root_dir
from utils import system

import patch

# These directories are snapshotted from chromium without modifications.
dirs_to_snapshot = [
    "base",
    "build",
    "cc",
    "gin",
    "gpu",
    "sandbox/linux",
    "sdch",
    "skia",
    "testing",
    "third_party/android_testrunner",
    "third_party/ashmem",
    "third_party/binutils",
    "third_party/boringssl",
    "third_party/brotli",
    "third_party/checkstyle",
    "third_party/cython",
    "third_party/fontconfig",
    "third_party/freetype2",
    "third_party/harfbuzz-ng",
    "third_party/iccjpeg",
    "third_party/jinja2",
    "third_party/jstemplate",
    "third_party/khronos",
    "third_party/libXNVCtrl",
    "third_party/libevent",
    "third_party/libjpeg",
    "third_party/libpng",
    "third_party/libxml",
    "third_party/markupsafe",
    "third_party/mesa",
    "third_party/modp_b64",
    "third_party/ply",
    "third_party/protobuf",
    "third_party/pymock",
    "third_party/qcms",
    "third_party/re2",
    "third_party/sfntly",
    "third_party/smhasher",
    "third_party/tcmalloc",
    "third_party/yasm",
    "third_party/zlib",
    "tools/android",
    "tools/clang",
    "tools/gdb",
    "tools/generate_library_loader",
    "tools/git",
    "tools/gritsettings",
    "tools/idl_parser",
    "tools/json_comment_eater",
    "tools/linux",
    "tools/lsan",
    "tools/msan",
    "tools/protoc_wrapper",
    "tools/relocation_packer",
    "tools/valgrind",
    "tools/vim",
    "tools/xdisplaycheck",
    "url",
    "ui/gl",
    ]

# These directories are temporarily cloned in order to support the network
# bootstrap library until we get that sorted out.
net_dirs = ["crypto",
    "net"]

files_to_copy = ["sandbox/sandbox_export.h",
    ".clang-format"]

dirs = dirs_to_snapshot + net_dirs

def chromium_rev_number(src_commit):
  base_url = "https://cr-rev.appspot.com/_ah/api/crrev/v1/commit/"
  commit_info = json.load(urllib2.urlopen(base_url + src_commit))
  return commit_info["numberings"][0]["number"]

def rev(source_dir):
  for d in dirs:
    print "removing directory %s" % d
    try:
      system(["git", "rm", "-r", d], cwd=mojo_root_dir)
    except subprocess.CalledProcessError:
      print "Could not remove %s" % d
    print "cloning directory %s" % d
    files = system(["git", "ls-files", d], cwd=source_dir)
    for f in files.splitlines():
      dest_path = os.path.join(mojo_root_dir, f)
      system(["mkdir", "-p", os.path.dirname(dest_path)], cwd=source_dir)
      system(["cp", os.path.join(source_dir, f), dest_path], cwd=source_dir)
    system(["git", "add", d], cwd=mojo_root_dir)

  for f in files_to_copy:
    system(["cp", os.path.join(source_dir, f), os.path.join(mojo_root_dir, f)])

  system(["git", "add", "."], cwd=mojo_root_dir)
  src_commit = system(["git", "rev-parse", "HEAD"], cwd=source_dir).strip()
  src_rev = chromium_rev_number(src_commit)
  commit("Update from https://crrev.com/" + src_rev, cwd=mojo_root_dir)

def main():
  parser = argparse.ArgumentParser(description="Update the mojo repo's " +
      "snapshot of things imported from chromium.")
  parser.add_argument("chromium_dir", help="chromium source dir")
  args = parser.parse_args()
  rev(args.chromium_dir)
  patch.patch()
  return 0

if __name__ == "__main__":
  sys.exit(main())
