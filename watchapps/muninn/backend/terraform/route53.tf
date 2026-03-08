resource "aws_apigatewayv2_domain_name" "domain_name" {
  domain_name = "muninn-api.${var.domain_name}"

  domain_name_configuration {
    certificate_arn = var.acm_certificate_arn
    endpoint_type   = "REGIONAL"
    security_policy = "TLS_1_2"
  }
}

# resource "aws_api_gateway_base_path_mapping" "api" {
#   api_id      = aws_apigatewayv2_api.history_api.id
#   stage_name  = "$default"
#   domain_name = aws_apigatewayv2_domain_name.domain_name.domain_name
# }

resource "aws_apigatewayv2_api_mapping" "api" {
  domain_name = aws_apigatewayv2_domain_name.domain_name.id
  api_id      = aws_apigatewayv2_api.history_api.id
  stage       = aws_apigatewayv2_stage.default.id
}

resource "aws_route53_record" "api_record" {
  zone_id = var.hosted_zone_id
  name    = "muninn-api.${var.domain_name}"
  type    = "A"

  alias {
    name                   = aws_apigatewayv2_domain_name.domain_name.domain_name_configuration[0].target_domain_name
    zone_id                = aws_apigatewayv2_domain_name.domain_name.domain_name_configuration[0].hosted_zone_id
    evaluate_target_health = false
  }
}
