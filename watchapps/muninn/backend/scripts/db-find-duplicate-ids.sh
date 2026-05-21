#!/bin/bash

aws dynamodb scan  --table-name ids  --projection-expression "watchToken, id" --region us-east-1 --output json | jq '
  .Items 
  | group_by(.watchToken.S) 
  | map(select(length > 1)) 
  | map({
      watchToken: .[0].watchToken.S, 
      duplicate_count: length, 
      ids: map(.id.S)
    })
'
