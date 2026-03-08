#!/bin/bash

set -eu

# 32 chars random
WATCH_TOKEN=${1:-$(openssl rand -hex 16)}
API_URL=https://muninn-api.chrislewis.me.uk

echo "URL $API_URL with watchToken: $WATCH_TOKEN"

RESPONSE=$(curl -s -X POST "$API_URL/id" \
  -H "Content-Type: application/json" \
  -d "{\"watchToken\": \"$WATCH_TOKEN\"}")
echo "Response: $RESPONSE"
