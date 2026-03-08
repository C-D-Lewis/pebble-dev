provider "aws" {
  region = "us-east-1"

  default_tags {
    tags = {
      Project   = "muninn-backend"
      ManagedBy = "Terraform"
    }
  }
}

terraform {
  required_version = "~> 1.14.0"

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 6.35.0"
    }
  }

  backend "s3" {
    bucket = "chrislewis-tfstate"
    key    = "muninn-backend"
    region = "us-east-1"
  }
}
