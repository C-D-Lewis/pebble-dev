output "domain_name_url" {
  value       = aws_apigatewayv2_domain_name.domain_name.domain_name
  description = "API custom domain URL"
}
