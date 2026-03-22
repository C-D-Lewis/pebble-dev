module "api" {
  source = "./api"

  region              = var.region
  project             = "muninn-backend"
  deletion_protection = var.deletion_protection
  hosted_zone_id      = var.hosted_zone_id
  domain_name         = var.domain_name
  acm_certificate_arn = var.acm_certificate_arn
}

module "client" {
  source = "github.com/c-d-lewis/terraform-modules//s3-cloudfront-website?ref=master"

  region              = "us-east-1"
  project_name        = "muninn-client"
  zone_id             = var.hosted_zone_id
  domain_name         = "muninn.${var.domain_name}"
  certificate_arn     = var.acm_certificate_arn
  default_root_object = "dist/index.html"
}

module "events" {
  source = "./events"

  project_name    = "muninn-events"
  api_lambda_arn  = module.api.api_lambda_arn
  api_lambda_name = module.api.api_lambda_name
}
