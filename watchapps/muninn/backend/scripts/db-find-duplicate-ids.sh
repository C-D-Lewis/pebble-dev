#!/bin/bash

RES=$(aws dynamodb scan  --table-name ids  --projection-expression "watchToken, id" --region us-east-1 --output json | jq '
  .Items 
  | group_by(.watchToken.S) 
  | map(select(length > 1)) 
  | map({
      watchToken: .[0].watchToken.S, 
      ids: map(.id.S)
    })
')
echo $RES

echo $RES > duplicates.json
