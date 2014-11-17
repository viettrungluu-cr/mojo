#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This a simple script to make building/testing Mojo components easier.

import argparse
import os
import platform
import re
import subprocess
import sys

import mopy.paths


def get_out_dir(args):
  out_dir = "out"
  prefix = ''
  if args.android:
    prefix = 'android_'
  elif args.chromeos:
    prefix = 'chromeos_'
  subdir = prefix + ('Debug' if args.debug else 'Release')
  return os.path.join(out_dir, subdir)


def sync(args):
  # pylint: disable=W0613
  return subprocess.call(['gclient', 'sync'])


def gn(args):
  command = ['gn', 'gen']

  gn_args = []
  gn_args.append('is_debug=' + ('true' if args.debug else 'false'))
  gn_args.append('is_asan=' + ('true' if args.asan else 'false'))
  gn_args.append('is_clang=' + ('true' if args.clang else 'false'))

  if platform.system() == 'Windows':
    # Force x64 for now to avoid .asm build problems
    gn_args.append('force_win64=true')

  goma_dir = os.environ.get('GOMA_DIR')
  goma_home_dir = os.path.join(os.getenv('HOME', ''), 'goma')
  if args.goma and goma_dir:
    gn_args.append('use_goma=true')
    gn_args.append(r'''goma_dir=\"%s\"''' % goma_dir)
  elif args.goma and os.path.exists(goma_home_dir):
    gn_args.append('use_goma=true')
    gn_args.append(r'''goma_dir=\"%s\"''' % goma_home_dir)
  else:
    gn_args.append('use_goma=false')

  if args.with_dart:
    gn_args.append('mojo_use_dart=true')

  if args.android:
    gn_args.append(r'''os=\"android\" cpu_arch=\"arm\"''')
  elif args.chromeos:
    gn_args.append(r'''os=\"chromeos\" ui_base_build_ime=false
                   use_system_harfbuzz=false''')

  out_dir = get_out_dir(args)
  command.append(out_dir)
  command.append('--args="%s"' % ' '.join(gn_args))

  print 'Running %s ...' % ' '.join(command)
  return subprocess.call(' '.join(command), shell=True)


def get_gn_arg_value(out_dir, arg):
  args_file_path = os.path.join(out_dir, "args.gn")
  if os.path.isfile(args_file_path):
    key_value_regex = re.compile(r'^%s = (.+)$' % arg)
    with open(args_file_path, "r") as args_file:
      for line in args_file.readlines():
        m = key_value_regex.search(line)
        if m:
          return m.group(1).strip('"')
  return ''


def build(args):
  out_dir = get_out_dir(args)
  print 'Building in %s ...' % out_dir
  if get_gn_arg_value(out_dir, 'use_goma') == 'true':
    # Use the configured goma directory.
    local_goma_dir = get_gn_arg_value(out_dir, 'goma_dir')
    print 'Ensuring goma (in %s) started ...' % local_goma_dir
    command = ['python',
               os.path.join(local_goma_dir, 'goma_ctl.py'),
               'ensure_start']
    exit_code = subprocess.call(command)
    if exit_code:
      return exit_code

    return subprocess.call(['ninja', '-j', '1000', '-l', '100', '-C', out_dir,
                            'root'])
  else:
    return subprocess.call(['ninja', '-C', out_dir, 'root'])


def run_unittests(args):
  out_dir = get_out_dir(args)
  print 'Running unit tests in %s ...' % out_dir
  command = ['python']
  if platform.system() == 'Linux':
    command.append('./testing/xvfb.py')
    command.append(out_dir)

  command.append(os.path.join('mojo', 'tools', 'test_runner.py'))
  command.append(os.path.join('mojo', 'tools', 'data', 'unittests'))
  command.append(out_dir)
  command.append('mojob_test_successes')
  return subprocess.call(command)


def run_skytests(args):
  out_dir = get_out_dir(args)
  if platform.system() != 'Linux':
    return 0

  command = []
  command.append('./testing/xvfb.py')
  command.append(out_dir)
  command.append('sky/tools/test_sky')
  command.append('-t')
  command.append('Debug' if args.debug else 'Release')
  command.append('--no-new-test-results')
  command.append('--no-show-results')
  command.append('--verbose')

  if args.builder_name:
    command.append('--builder-name')
    command.append(args.builder_name)

  if args.build_number:
    command.append('--build-number')
    command.append(args.build_number)

  if args.master_name:
    command.append('--master-name')
    command.append(args.master_name)

  if args.test_results_server:
    command.append('--test-results-server')
    command.append(args.test_results_server)

  return subprocess.call(command)


def run_pytests(args):
  out_dir = get_out_dir(args)
  print 'Running python tests in %s ...' % out_dir
  command = ['python']
  command.append(os.path.join('mojo', 'tools', 'run_mojo_python_tests.py'))
  exit_code = subprocess.call(command)
  if exit_code:
    return exit_code

  if platform.system() != 'Linux':
    print ('Python bindings tests are only supported on Linux.')
    return

  command = ['python']
  command.append(os.path.join('mojo', 'tools',
                              'run_mojo_python_bindings_tests.py'))
  command.append('--build-dir=' + out_dir)
  return subprocess.call(command)


def test(args):
  exit_code = run_unittests(args)
  if exit_code:
    return exit_code
  exit_code = run_pytests(args)
  if exit_code:
    return exit_code
  return run_skytests(args)


def perftest(args):
  out_dir = get_out_dir(args)
  print 'Running perf tests in %s ...' % out_dir
  command = []
  command.append(os.path.join(out_dir, 'mojo_public_system_perftests'))
  return subprocess.call(command)


def pytest(args):
  return run_pytests(args)


def darttest(args):
  out_dir = get_out_dir(args)
  print 'Running Dart tests in %s ...' % out_dir
  command = []
  command.append('dart')
  command.append('--checked')
  command.append('--enable-async')
  command.append(os.path.join('mojo', 'tools', 'dart_test_runner.dart'))
  command.append(os.path.join(out_dir, 'gen'))
  return subprocess.call(command)


def main():
  os.chdir(mopy.paths.Paths().src_root)

  parser = argparse.ArgumentParser(description='A script to make building'
      '/testing Mojo components easier.')

  parent_parser = argparse.ArgumentParser(add_help=False)
  debug_group = parent_parser.add_mutually_exclusive_group()
  debug_group.add_argument('--debug', help='Debug build (default)',
                           default=True, action='store_true')
  debug_group.add_argument('--release', help='Release build', default=False,
                           dest='debug', action='store_false')

  os_group = parent_parser.add_mutually_exclusive_group()
  os_group.add_argument('--android', help='Build for Android',
                        action='store_true')
  os_group.add_argument('--chromeos', help='Build for ChromeOS',
                        action='store_true')

  subparsers = parser.add_subparsers()

  sync_parser = subparsers.add_parser('sync', parents=[parent_parser],
      help='Sync using gclient (does not run gn).')
  sync_parser.set_defaults(func=sync)

  gn_parser = subparsers.add_parser('gn', parents=[parent_parser],
                                    help='Run gn for mojo (does not sync).')
  gn_parser.set_defaults(func=gn)
  gn_parser.add_argument('--asan', help='Uses Address Sanitizer',
                         action='store_true')
  gn_parser.add_argument('--with-dart', help='Configure the Dart bindings',
                         action='store_true')
  clang_group = gn_parser.add_mutually_exclusive_group()
  clang_group.add_argument('--clang', help='Use Clang (default)', default=True,
                           action='store_true')
  clang_group.add_argument('--gcc', help='Use GCC', default=False,
                           dest='clang', action='store_false')
  goma_group = gn_parser.add_mutually_exclusive_group()
  goma_group.add_argument('--goma',
                          help='Use Goma (if $GOMA_DIR is set or $HOME/goma '
                              'exists;default)',
                          default=True,
                          action='store_true')
  goma_group.add_argument('--no-goma', help='Don\'t use Goma', default=False,
                          dest='goma', action='store_false')

  build_parser = subparsers.add_parser('build', parents=[parent_parser],
                                       help='Build')
  build_parser.set_defaults(func=build)

  test_parser = subparsers.add_parser('test', parents=[parent_parser],
                                      help='Run unit tests (does not build).')
  test_parser.set_defaults(func=test)

  # Arguments required for uploading to the flakiness dashboard.
  test_parser.add_argument('--master-name',
      help='The name of the buildbot master.')
  test_parser.add_argument('--builder-name',
      help=('The name of the builder shown on the waterfall running '
            'this script e.g. Mojo Linux.'))
  test_parser.add_argument('--build-number',
      help='The build number of the builder running this script.')
  test_parser.add_argument('--test-results-server',
      help='Upload results json files to this appengine server.')

  perftest_parser = subparsers.add_parser('perftest', parents=[parent_parser],
      help='Run perf tests (does not build).')
  perftest_parser.set_defaults(func=perftest)

  pytest_parser = subparsers.add_parser('pytest', parents=[parent_parser],
      help='Run Python unit tests (does not build).')
  pytest_parser.set_defaults(func=pytest)

  darttest_parser = subparsers.add_parser('darttest', parents=[parent_parser],
      help='Run Dart unit tests (does not build).')
  darttest_parser.set_defaults(func=darttest)

  args = parser.parse_args()

  # Android always wants GCC.
  if args.android:
    args.clang = False

  if platform.system() == 'Windows':
    args.clang = False

  return args.func(args)


if __name__ == '__main__':
  sys.exit(main())
