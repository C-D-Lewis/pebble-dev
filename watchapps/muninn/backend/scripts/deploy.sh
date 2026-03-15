#!/bin/bash

# Simplified version of the common one

set -eu

export AWS_DEFAULT_REGION=us-east-1

# Website domain
SITE_DOMAIN=muninn.chrislewis.me.uk
# Bucket created by Terraform
BUCKET="s3://$SITE_DOMAIN"
# Current commit
COMMIT=$(git rev-parse --short HEAD)

############################################ Build site ############################################

cd client

printf "\n\n>>> Building site\n\n"
npm run build

############################################ Push site #############################################

printf "\n\n>>> Pushing client files\n\n"

aws s3 cp index.html $BUCKET
aws s3 sync dist $BUCKET/dist || true
aws s3 sync assets $BUCKET/assets || true

cd -

########################################## Update infra ############################################

printf "\n\n>>> Updating infrastructure\n\n"

cd terraform
terraform init
terraform apply -auto-approve

cd -

############################################ Test API ##############################################

cd lambda
npm test

cd -

########################################## Invalidate ##############################################

printf "\n\n>>> Invalidating CloudFront\n\n"

CF_DIST_ID=$(aws cloudfront list-distributions | jq -r ".DistributionList.Items[] | select(.Aliases.Items | tostring | contains(\"$SITE_DOMAIN\")) | .Id")
RES=$(aws cloudfront create-invalidation --distribution-id $CF_DIST_ID --paths "/*")
INVALIDATION_ID=$(echo $RES | jq -r '.Invalidation.Id')

# Wait for invalidation complete
echo "Waiting for invalidation-completed for $INVALIDATION_ID..."
aws cloudfront wait invalidation-completed --distribution-id $CF_DIST_ID --id $INVALIDATION_ID

printf "\n\n>>> Deployment complete!\n\n"
