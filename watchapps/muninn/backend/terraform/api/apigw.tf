resource "aws_apigatewayv2_api" "history_api" {
  name          = "${var.project}-apigw"
  protocol_type = "HTTP"
}

resource "aws_apigatewayv2_integration" "integration" {
  api_id                 = aws_apigatewayv2_api.history_api.id
  integration_type       = "AWS_PROXY"
  integration_uri        = aws_lambda_function.api_lambda.invoke_arn
  payload_format_version = "2.0"
}

resource "aws_apigatewayv2_route" "post_upload" {
  api_id    = aws_apigatewayv2_api.history_api.id
  route_key = "POST /history"
  target    = "integrations/${aws_apigatewayv2_integration.integration.id}"
}

resource "aws_apigatewayv2_route" "post_id" {
  api_id    = aws_apigatewayv2_api.history_api.id
  route_key = "POST /id"
  target    = "integrations/${aws_apigatewayv2_integration.integration.id}"
}

resource "aws_apigatewayv2_route" "get_history" {
  api_id    = aws_apigatewayv2_api.history_api.id
  route_key = "GET /history/{id}"
  target    = "integrations/${aws_apigatewayv2_integration.integration.id}"
}

resource "aws_apigatewayv2_route" "get_stats" {
  api_id    = aws_apigatewayv2_api.history_api.id
  route_key = "GET /globalStats"
  target    = "integrations/${aws_apigatewayv2_integration.integration.id}"
}

resource "aws_apigatewayv2_route" "post_recompute" {
  api_id    = aws_apigatewayv2_api.history_api.id
  route_key = "POST /recompute"
  target    = "integrations/${aws_apigatewayv2_integration.integration.id}"
}

resource "aws_apigatewayv2_stage" "default" {
  api_id      = aws_apigatewayv2_api.history_api.id
  name        = "$default"
  auto_deploy = true

  access_log_settings {
    destination_arn = aws_cloudwatch_log_group.api_gw.arn
    format = jsonencode({
      requestId = "$context.requestId",
      ip        = "$context.identity.sourceIp",
      routeKey  = "$context.routeKey",
      status    = "$context.status"
    })
  }
}

resource "aws_cloudwatch_log_group" "api_gw" {
  name              = "/aws/api-gw/${var.project}-history-api"
  retention_in_days = 7
}
