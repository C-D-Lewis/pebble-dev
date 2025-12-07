#!/bin/bash

set -eu

COMMAND=${1:-pebble}
INITIAL=$(pwd)

# Projects expected to build successfully
working_projects=$(cat <<EOF
./libraries/pebble-packet/test-app/
./libraries/pebble-universal-fb/test-app/
./libraries/pebble-isometric/test-app/
./libraries/pebble-pge/test-app/
./libraries/pebble-scalable/test-app/
./libraries/notif-layer/test-app/
./libraries/InverterLayerCompat/test-app/

./watchfaces/beam-up/
./watchfaces/brackets/
./watchfaces/cards/
./watchfaces/cmd-time/
./watchfaces/cmd-time-typed/
./watchfaces/dayring/
./watchfaces/deep-rock/
./watchfaces/dual-gauge/
./watchfaces/eclipse/
./watchfaces/eventful/
./watchfaces/events/
./watchfaces/hollywatch/
./watchfaces/index/
./watchfaces/isotime-appstore/
./watchfaces/kitty-watchface/
./watchfaces/morndas/
./watchfaces/past-present-future/
./watchfaces/potential-divider/
./watchfaces/pseudotime/
./watchfaces/split-horizon/
./watchfaces/starfield-smooth/
./watchfaces/starfield-demo/
./watchfaces/thin/
./watchfaces/time-dots-appstore/
./watchfaces/void-statues/

./watchapps/block-world/
./watchapps/dashboard/pebble/
./watchapps/news-headlines/
./watchapps/tube-status/
./watchapps/muninn/
EOF
)

function build_project {
  echo ">>> Building $1"
  cd $1

  # Pebble packages will require dist populating on CI
  if [[ "$1" == *pebble-* ]]; then
    echo $PWD
    echo ">>> Detected pebble package — building parent directory first"
    cd ..
    $COMMAND build
    cd -
  fi

  $COMMAND clean
  $COMMAND build
  echo ">>> Exit code for $1: $?"

  cd $INITIAL
}

function setup {
  echo ">>> Setting up build environment..."
  
  # Backup cards secrets.js and replace with dummy version
  mv ./watchfaces/cards/src/pkjs/secrets.js ./watchfaces/cards/src/pkjs/secrets.js.bak || true
  echo "module.exports = { token: 'foo' };" > ./watchfaces/cards/src/pkjs/secrets.js

  # Populate build files for library test apps
  cp ./libraries/pebble-isometric/include/pebble-isometric.h ./libraries/pebble-isometric/test-app/src/c/pebble-isometric.h
  cp ./libraries/pebble-isometric/src/c/pebble-isometric.c ./libraries/pebble-isometric/test-app/src/c/pebble-isometric.c
}

function build_all_projects {
  for dir in $working_projects; do
    build_project "$dir"
  done

  echo ">>> Completed build_all_projects"
}

setup
build_all_projects
