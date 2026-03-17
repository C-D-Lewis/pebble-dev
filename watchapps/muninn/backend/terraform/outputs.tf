output "domain_name_url" {
  value       = module.api.domain_name_url
  description = "API custom domain URL"
}

output "distribution_id" {
  value = module.client.distribution_id
}
