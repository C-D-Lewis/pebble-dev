#!/bin/bash

set -eu

# 32 chars random
ID=$1
API_URL=$(terraform output api_gateway_url | tr -d '"')

echo "URL $API_URL with id: $ID"

RESPONSE=$(curl -s -X GET "$API_URL/history/$ID")
echo "Response: $RESPONSE"
