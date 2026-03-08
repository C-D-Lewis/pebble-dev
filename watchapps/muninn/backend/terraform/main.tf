module "main" {
  source = "./infrastructure"

  region              = "us-east-1"
  project             = "muninn-backend"
  deletion_protection = false
  hosted_zone_id      = "Z05682866H59A0KFT8S"
  domain_name         = "chrislewis.me.uk"
  acm_certificate_arn = "arn:aws:acm:us-east-1:617929423658:certificate/a69e6906-579e-431d-9e4c-707877d325b7"
}
