#!/bin/bash

# Intended for use with a Docker image named 'pebble'

set -eu

docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $(pwd):/pebble/ \
  pebble-sdk
