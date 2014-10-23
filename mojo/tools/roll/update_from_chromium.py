#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import patch
import rev
import sys

if len(sys.argv) != 2:
  print "usage: updatemojo.py <chromium source dir>"
  sys.exit(1)

chromium_dir = sys.argv[1]

print "Updating from %s" % chromium_dir

rev.rev(chromium_dir)
patch.patch()
