# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

def gtest_list_tests(gtest_list_tests_output):
  """Returns a list of strings formatted as TestSuite.TestFixture from the
  output of running --gtest_list_tests on a GTEST application."""

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
