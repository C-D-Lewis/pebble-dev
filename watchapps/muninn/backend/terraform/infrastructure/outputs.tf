output "api_gateway_url" {
  value       = aws_apigatewayv2_stage.default.invoke_url
  description = "The public endpoint for the API GW"
}

output "domain_name_url" {
  value       = aws_apigatewayv2_domain_name.domain_name.domain_name
  description = "API custom domain URL"
}
