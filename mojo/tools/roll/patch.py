#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import utils

def patch():
  os.chdir(utils.mojo_root_dir)

  utils.filter_file("build/landmines.py",
      lambda line: not "gyp_environment" in line)
  utils.commit("filter gyp_environment out of build/landmines.py")

  utils.filter_file("gpu/BUILD.gn", lambda line: not "//gpu/ipc" in line)
  utils.commit("filter gpu/ipc out of gpu/BUILD.gn")

  for p in utils.find(["*.patch"], os.path.dirname(os.path.realpath(__file__))):
    print "applying patch %s" % os.path.basename(p)
    utils.system(["git", "apply", p])
    utils.commit("applied patch %s" % os.path.basename(p))
