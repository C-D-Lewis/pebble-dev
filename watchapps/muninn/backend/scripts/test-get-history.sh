#!/bin/bash

set -eu

API_URL=https://muninn-api.chrislewis.me.uk

ID=$1

RESPONSE=$(curl -s -X GET "$API_URL/history/$ID")
echo $RESPONSE | jq
