# backend

This is the backend to Muninn's web reporting feature. Components are:

* DynamoDB database
* API Gateway API
* Lambda API handler
* S3 bucket website
* CloudFront distribution
* Route53 record in hosted zone

> The sharing of watch charge history samples is voluntary and Muninn can
> provide 90% of its value without ever using this feature.

## Overview

Data is send to the API when the user selects the upload option in the webapp
and then retrieved by the client when provided the exchanged code. The workflow
is:

1. Muninn records samples and sends them to the JS side for longer-term storage.
2. The user selects the upload menu option.
3. The JS side uses an ID exchanged for the unique watch token to submit the
   history data and basic stats to the API.
4. The API validates and stores the data in DynamoDB.
5. The user is instructed to open the watchapp config page in the Pebble app.
6. The config page URL is the client URL, with the exchanged code in the query
   params.
7. The client webapp reads the watch's history from the API and displays chart
   and statistics.

## Deploy

Install:

* AWS CLI
* Terraform

Setup AWS credentials that can manage the DynamoDB, Lambda, API Gateway, S3,
CloudFront, and Route53 APIs required.

Install dependencies in the `client` and `lambda` directories:

```shell
npm install
```

Finally, run the deploy script - Terraform manages the Lambda TypeScript build,
and the static site assets are built with Vite and uploaded to S3.

```shell
./scripts/deploy.sh
```

## TODO

* If possible and sample size is large enough, show some stats per-model or age.
