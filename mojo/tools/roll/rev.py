#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
from utils import commit
from utils import mojo_root_dir
from utils import system

# These directories are snapshotted from chromium without modifications.
dirs_to_snapshot = [
    "base",
    "build",
    "cc",
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
    "gin",
    "net",
    ]

files_to_copy = ["sandbox/sandbox_export.h"]

dirs = dirs_to_snapshot + net_dirs

def rev(source_dir):
  os.chdir(source_dir)
  src_commit = system(["git", "show-ref", "HEAD"])

  for d in dirs:
    print "removing directory %s" % d
    os.chdir(mojo_root_dir)
    try:
      system(["git", "rm", "-r", d])
    except subprocess.CalledProcessError:
      print "Could not remove %s" % d
    print "cloning directory %s" % d
    os.chdir(source_dir)
    files = system(["git", "ls-files", d])
    for f in files.splitlines():
      dest_path = os.path.join(mojo_root_dir, f)
      system(["mkdir", "-p", os.path.dirname(dest_path)])
      system(["cp", os.path.join(source_dir, f), dest_path])
    os.chdir(mojo_root_dir)
    system(["git", "add", d])

  for f in files_to_copy:
    system(["cp", os.path.join(source_dir, f), os.path.join(mojo_root_dir, f)])

  os.chdir(mojo_root_dir)
  system(["git", "add", "."])
  commit("Update from chromium " + src_commit)
