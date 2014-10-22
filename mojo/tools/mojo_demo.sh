#!/bin/bash
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This a simple script to make launching demos easier.

DIRECTORY="$(dirname "$0")"/../../out/Debug
PORT=$(($RANDOM % 8192 + 2000))

do_help() {
  cat << EOF
Usage: $(basename "$0") [-d DIRECTORY] --browser|--wm_flow

DIRECTORY defaults to $DIRECTORY.

Example:
  $(basename "$0") --browser
EOF
}

args=

while [ $# -gt 0 ]; do
  case "$1" in
    -h|--help)
      do_help
      exit 0
      ;;
    -d)
      shift
      if [ $# -eq 0 ]; then
        do_help
        exit 1
      fi
      DIRECTORY="$1"
      ;;
    --browser)
      args="--url-mappings=mojo:window_manager=mojo:example_window_manager "
      args+="mojo:window_manager"
      ;;
    --wm_flow)
      args="--url-mappings=mojo:window_manager=mojo:wm_flow_wm mojo:wm_flow_app"
      ;;
    *)
      break
      ;;
  esac
  shift
done

if [ -z "$args" ]; then
  do_help
  exit 0
fi

echo "Base directory: $DIRECTORY"

echo "$DIRECTORY/mojo_shell $args"
echo $args
$DIRECTORY/mojo_shell $args
