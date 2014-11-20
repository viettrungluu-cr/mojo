# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from mopy.gtest_list_tests import gtest_list_tests

class GTestListTestsTest(unittest.TestCase):
  """Tests |gtest_list_tests()| handling of --gtest_list_tests output."""

  def testSingleSuiteAndFixture(self):
    """Tests a single suite with a single fixture."""
    gtest_output = "TestSuite.\n  TestFixture1"
    expected_test_list = ["TestSuite.TestFixture1"]
    self.assertEquals(gtest_list_tests(gtest_output), expected_test_list)

  def testExtraBlankLinesAndSpaces(self):
    """Tests handling of extra blank lines and extra spaces before fixtures."""
    gtest_output = "TestSuite.\n\n     TestFixture1\n\n"
    expected_test_list = ["TestSuite.TestFixture1"]
    self.assertEquals(gtest_list_tests(gtest_output), expected_test_list)

  def testWindowsNewlines(self):
    """Tests handling of \r\n newlines."""
    gtest_output = "TestSuite.\r\n  TestFixture1\r\n"
    expected_test_list = ["TestSuite.TestFixture1"]
    self.assertEquals(gtest_list_tests(gtest_output), expected_test_list)

  def testSingleSuiteAndMultipleFixtures(self):
    """Tests a single suite with multiple fixtures."""
    gtest_output = "TestSuite.\n  TestFixture1\n  TestFixture2"
    expected_test_list = ["TestSuite.TestFixture1", "TestSuite.TestFixture2"]
    self.assertEquals(gtest_list_tests(gtest_output), expected_test_list)

  def testMultipleSuitesAndFixtures(self):
    """Tests multiple suites each with multiple fixtures."""
    gtest_output = ("TestSuite1.\n  TestFixture1\n  TestFixture2\n"
                    "TestSuite2.\n  TestFixtureA\n  TestFixtureB\n")
    expected_test_list = ["TestSuite1.TestFixture1", "TestSuite1.TestFixture2",
                          "TestSuite2.TestFixtureA", "TestSuite2.TestFixtureB"]
    self.assertEquals(gtest_list_tests(gtest_output), expected_test_list)

if __name__ == "__main__":
  unittest.main()
