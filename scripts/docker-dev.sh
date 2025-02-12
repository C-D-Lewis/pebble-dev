#!/bin/bash

set -eu

DIR=$1

cd $DIR && docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $(pwd):/pebble/ \
  pebble-sdk
