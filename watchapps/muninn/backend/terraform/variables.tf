variable "region" {
  type        = string
  description = "Region to deploy to"
  default = "us-east-1"
}

variable "deletion_protection" {
  type        = string
  description = "Whether DynamoDB deletion protection is enabled"
  default = true
}

variable "hosted_zone_id" {
  type        = string
  description = "The ID of the existing Route 53 hosted zone"
  default = "Z05682866H59A0KFT8S"
}

variable "domain_name" {
  type        = string
  description = "Domain name the API belongs to"
  default = "chrislewis.me.uk"
}

variable "acm_certificate_arn" {
  type        = string
  description = "ARN of the ACM certificate for the domain"
  default = "arn:aws:acm:us-east-1:617929423658:certificate/a69e6906-579e-431d-9e4c-707877d325b7"
}
