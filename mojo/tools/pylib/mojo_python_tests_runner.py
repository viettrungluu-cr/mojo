# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys
import unittest


class MojoPythonTestRunner(object):
  """Helper class to run python tests on the bots."""

  def __init__(self, test_dir):
    self._test_dir = test_dir

  def run(self):
    parser = argparse.ArgumentParser()
    parser.usage = 'run_mojo_python_tests.py [options] [tests...]'
    parser.add_argument('-v', '--verbose', action='count', default=0)
    parser.add_argument('tests', nargs='*')

    self.add_custom_commandline_options(parser)
    args = parser.parse_args()
    self.apply_customization(args)

    chromium_src_dir = os.path.join(os.path.dirname(__file__),
                                    os.pardir,
                                    os.pardir,
                                    os.pardir)

    loader = unittest.loader.TestLoader()
    print "Running Python unit tests under %s..." % self._test_dir

    pylib_dir = os.path.abspath(os.path.join(chromium_src_dir, self._test_dir))
    if args.tests:
      if pylib_dir not in sys.path:
        sys.path.append(pylib_dir)
      suite = unittest.TestSuite()
      for test_name in args.tests:
        suite.addTests(loader.loadTestsFromName(test_name))
    else:
      suite = loader.discover(pylib_dir, pattern='*_unittest.py')

    runner = unittest.runner.TextTestRunner(verbosity=(args.verbose + 1))
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1

  def add_custom_commandline_options(self, parser):
    """Allow to add custom option to the runner script."""
    pass

  def apply_customization(self, args):
    """Allow to apply any customization to the runner."""
    pass
