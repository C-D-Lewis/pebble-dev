output "api_gateway_url" {
  value       = module.main.api_gateway_url
  description = "The public endpoint for your API"
}

output "domain_name_url" {
  value       = module.main.domain_name_url
  description = "API custom domain URL"
}
