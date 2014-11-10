#!/bin/bash
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

python "$(dirname "$0")/mojob.py" "$@"
echo ""
echo ""
echo "NOTE:  mojob.sh is deprecated, use mojob.py instead. The arguments also changed order, see mojob.py --help"