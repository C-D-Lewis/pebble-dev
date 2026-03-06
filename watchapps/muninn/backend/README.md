# backend

This is the backend to Muninn's web reporting feature.

Currently a work in progress, the aims are:

* Opt-in setting in the watchapp to upload anonymized watch history.
* API Gateway to receive uploads and return them to the web UI.
* DynamoDB tables for anonymous IDs (correlating to watch tokens), and storing
  uploaded sample histories.
* Web UI that shows a graph of all that watch's data in detail, as well as some
  summary stats.
* If possible and sample size is large enough, show some stats per-model or age.
