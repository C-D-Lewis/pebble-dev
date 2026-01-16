#!/bin/bash

# Script to allow building either Tube Status (TfL only) or Transit Status (many backends)

set -eu

APP_NAME=$1

# Check it is one of the allowed values
if [[ "$APP_NAME" != "tubestatus" && "$APP_NAME" != "transitstatus" ]]; then
    echo "Error: Invalid application name '$APP_NAME'."
    echo "Usage: $0 {tubestatus|transitstatus}"
    exit 1
fi

echo ">>> Switching to $APP_NAME"
rm package.json || true
rm package-lock.json || true
cp package.$APP_NAME.json package.json

echo ">>> Building app"
npm i
pebble clean && pebble build
echo ">>> Complete!"
