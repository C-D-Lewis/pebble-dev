#!/bin/bash

# Simplified version of the usual one

set -eu

export AWS_DEFAULT_REGION=us-east-1

# Website domain
SITE_DOMAIN=muninn.chrislewis.me.uk
# Bucket created by Terraform
BUCKET="s3://$SITE_DOMAIN"
# Current commit
COMMIT=$(git rev-parse --short HEAD)

############################################### Build ##############################################

printf "\n\n>>> Building site\n\n"
npm run build
  
# Check no changes resulted
# if [[ ! -z $(git status -s) ]]; then
#   echo "There are uncommitted changes after build!"
#   exit 1
# fi

############################################### Push ###############################################

printf "\n\n>>> Pushing files\n\n"

aws s3 cp index.html $BUCKET
aws s3 sync dist $BUCKET/dist || true
aws s3 sync assets $BUCKET/assets || true

# For infrastructure, run Terraform manually

printf "\n\n>>> Deployment complete!\n\n"
