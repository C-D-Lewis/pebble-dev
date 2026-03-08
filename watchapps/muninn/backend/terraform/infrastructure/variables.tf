variable "region" {
  type        = string
  description = "Region to deploy to"
}

variable "project" {
  type        = string
  description = "Name of the project"
}

variable "deletion_protection" {
  type        = string
  description = "Whether DynamoDB deletion protection is enabled"
}

variable "hosted_zone_id" {
  type        = string
  description = "The ID of the existing Route 53 hosted zone"
}

variable "domain_name" {
  type        = string
  description = "Domain name the API belongs to"
}

variable "acm_certificate_arn" {
  type        = string
  description = "ARN of the ACM certificate for the domain"
}
