resource "aws_cloudwatch_event_rule" "daily_trigger" {
  name                = "${var.project_name}-lambda-trigger"
  description         = "Trigger the API Lambda once a day"
  schedule_expression = "cron(0 0 * * ? *)" 
}

resource "aws_cloudwatch_event_target" "lambda_target" {
  rule      = aws_cloudwatch_event_rule.daily_trigger.name
  target_id = "SendToLambda"
  arn       = var.api_lambda_arn

  # Optional: Pass a custom constant JSON to the 'detail' field
  input = jsonencode({
    "detail-type": "DailyAggregation",
    "source": "aws.events"
  })
}
