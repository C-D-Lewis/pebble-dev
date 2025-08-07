#!/bin/bash

set -eu

COMMAND=${1:-pebble}

# Projects expected to build successfully
working_projects=$(cat <<EOF
./watchfaces/brackets/
./watchfaces/cards/
./watchfaces/cmd-time/
./watchfaces/dayring/
./watchfaces/deep-rock/
./watchfaces/dual-gauge/
./watchfaces/eclipse/
./watchfaces/hollywatch/
./watchfaces/index/
./watchfaces/kitty-watchface/
./watchfaces/morndas/
./watchfaces/past-present-future/
./watchfaces/pseudotime/
./watchfaces/split-horizon-pe/
./watchfaces/starfield-smooth/
./watchfaces/startfield-demo/
./watchfaces/timedots-appstore/

./watchapps/block-world/
./watchapps/module-app-base/
./watchapps/news-headlines/
./watchapps/tube-status/
EOF
)

function build_project {
  if echo "$working_projects" | grep -q ".*$1.*"; then
    echo "Building $1"
    cd $1
    $COMMAND clean
    $COMMAND build
    echo "Exit code for $1: $?"

    # Copy the build - note that generatedAt field means they will be new in git
    name=$(basename $1)
    cp "./build/$name.pbw" "../../pbw/$name.pbw"
    cd ../..
  else
    echo "Skipping $1"
  fi
}

function build_all_in_dir {
  for dir in $1/*/; do
    build_project "$dir"
  done
}

build_all_in_dir ./watchfaces
build_all_in_dir ./watchapps
