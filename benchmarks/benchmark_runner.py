#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import imp
import importlib
import os
import sys

try:
  imp.find_module('mopy')
except ImportError:
  sys.path.append(os.path.abspath(os.path.join(
      __file__, os.pardir, os.pardir, 'mojo', 'tools')))
from mopy.paths import Paths


class BenchmarkRunner(object):
  def __init__(self, args):
    self._args = args
    self._benchmark_dir = os.path.dirname(os.path.realpath(__file__))
    if args.release:
      build_directory = os.path.join('out', 'Release')
    else:
      build_directory = os.path.join('out', 'Debug')
    self._paths = Paths(build_directory)

  def _list_tests(self):
    for name in os.listdir(self._benchmark_dir):
      path = os.path.join(self._benchmark_dir, name)
      if os.path.isdir(path):
        yield name

  def _run_test(self, test_name):
    print "Running %s ..." % test_name
    run_script_path = os.path.join(self._benchmark_dir, test_name, 'run.py')
    if os.path.isfile(run_script_path):
      run_module = '.'.join([test_name, 'run'])
      importlib.import_module(run_module)
      result = sys.modules[run_module].run(self._args, self._paths)

      #TODO(yzshen): (1) consider using more structured result;
      #              (2) upload the result to server.
      print result

  def run(self):
    for test in self._list_tests():
      self._run_test(test)


def main():
  parser = argparse.ArgumentParser(
      description='Mojo performance benchmark runner')

  debug_group = parser.add_mutually_exclusive_group()
  debug_group.add_argument('--release',
                           help='test against release build (default)',
                           default=True, action='store_true')
  debug_group.add_argument('--debug', help='test against debug build',
                           default=False, dest='release', action='store_false')

  args = parser.parse_args()

  BenchmarkRunner(args).run()


if __name__ == '__main__':
  sys.exit(main())
