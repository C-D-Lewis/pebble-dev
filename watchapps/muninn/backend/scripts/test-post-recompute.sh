#!/bin/bash

set -eu

API_URL=https://muninn-api.chrislewis.me.uk

RESPONSE=$(curl -s -X POST "$API_URL/recompute")
echo "Response: $RESPONSE"
