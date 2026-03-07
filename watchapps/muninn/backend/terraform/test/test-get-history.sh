#!/bin/bash

set -eu

ID=$1

API_URL=https://muninn-api.chrislewis.me.uk

echo "URL $API_URL with id: $ID"

RESPONSE=$(curl -s -X GET "$API_URL/history/$ID")
echo "Response: $RESPONSE"
