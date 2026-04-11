#!/bin/bash

set -eu

aws dynamodb scan \
  --table-name history \
  --max-items 5000 \
  --region us-east-1 > aggregations.json
