#!/bin/bash

set -e

echo ">>> Installing dependencies"
sudo apt-get update && sudo apt-get install -y \
  curl \
  libsdl1.2debian \
  libfdt1 \
  python3.12-venv \
  gcc-arm-none-eabi

echo ">>> Installing uv"
curl -LsSf https://astral.sh/uv/install.sh | sh

echo ">>> Installing pebble tool"
uv tool install pebble-tool

echo ">>> Tetsing pebble tool"
pebble new-project test
cd test && pebble build

echo ">>> Tool installed successfully"
