#!/bin/bash

function build_watchface {
  (cd "$1" && pebble build)
  echo "Exit code for $1: $?"
}

function build_all_in_dir {
  for dir in $1/*/; do
    build_watchface "$dir"
  done
}

build_all_in_dir ./watchfaces
build_all_in_dir ./watchapps
