#!/bin/bash

set -eu

API_URL=https://muninn-api.chrislewis.me.uk

RESPONSE=$(curl -s -X GET "$API_URL/globalStats")
echo $RESPONSE
