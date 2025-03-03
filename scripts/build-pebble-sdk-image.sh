#!/bin/bash

set -eu

git clone https://github.com/C-D-Lewis/docker-pebble-dev
cd docker-pebble-dev
docker build -t pebble-sdk .

cd ..
rm -rf docker-pebble-dev
