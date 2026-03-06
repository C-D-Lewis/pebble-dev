#!/bin/bash

set -eu

# 32 chars random
ID=$1
HISTORY_FILE=${2:-./test/example.json}
API_URL=$(terraform output api_gateway_url | tr -d '"')

echo "URL $API_URL with history: $HISTORY_FILE"

HISTORY_JSON=$(cat "$HISTORY_FILE")
RESPONSE=$(curl -s -X POST "$API_URL/history" \
  -H "Content-Type: application/json" \
  -d "{\"id\": \"$ID\", \"history\": $HISTORY_JSON}")
echo "Response: $RESPONSE"
