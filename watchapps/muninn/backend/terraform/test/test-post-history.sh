#!/bin/bash

set -eu

API_URL=https://muninn-api.chrislewis.me.uk

# 32 chars random
ID=$1
HISTORY_FILE=${2:-./test/example.json}

HISTORY_JSON=$(cat "$HISTORY_FILE")

RESPONSE=$(curl -s -X POST "$API_URL/history" \
  -H "Content-Type: application/json" \
  -d "{\"id\": \"$ID\", \"history\": $HISTORY_JSON}")
echo "Response: $RESPONSE"
