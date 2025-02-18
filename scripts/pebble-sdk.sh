#!/bin/bash

set -eu

docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $(pwd):/pebble/ \
  pebble-sdk pebble $@
