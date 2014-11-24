# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re


def gtest_list_tests(gtest_list_tests_output):
  """Returns a list of strings formatted as TestSuite.TestFixture from the
  output of running --gtest_list_tests on a GTEST application."""

  if not re.match("^(\w*\.\r?\n(  \w*\r?\n)+)+", gtest_list_tests_output):
    raise Exception("Unrecognized --gtest_list_tests output:\n%s" %
                    gtest_list_tests_output)

  output_lines = gtest_list_tests_output.split('\n')

  test_list = []
  for line in output_lines:
    if not line:
      continue
    if line[0] != ' ':
      suite = line.strip()
      continue
    test_list.append(suite + line.strip())

  return test_list
