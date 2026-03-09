#!/bin/bash

set -eu

API_URL=https://muninn-api.chrislewis.me.uk

echo "URL $API_URL for stats"

RESPONSE=$(curl -s -X GET "$API_URL/stats")
echo "Response: $RESPONSE"
