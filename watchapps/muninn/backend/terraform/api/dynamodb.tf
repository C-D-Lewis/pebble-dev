resource "aws_dynamodb_table" "ids_table" {
  name                        = "ids"
  billing_mode                = "PROVISIONED"
  read_capacity               = 5
  write_capacity              = 5
  hash_key                    = "id"
  deletion_protection_enabled = var.deletion_protection

  point_in_time_recovery {
    enabled = true
    recovery_period_in_days = 14
  }

  attribute {
    name = "id"
    type = "S"
  }

  attribute {
    name = "watchToken"
    type = "S"
  }

  global_secondary_index {
    name            = "WatchTokenIndex"
    projection_type = "ALL"
    read_capacity   = 5
    write_capacity  = 5

    key_schema {
      attribute_name = "watchToken"
      key_type       = "HASH"
    }
  }
}

resource "aws_dynamodb_table" "history_table" {
  name                        = "history"
  billing_mode                = "PROVISIONED"
  read_capacity               = 5
  write_capacity              = 5
  hash_key                    = "id"
  deletion_protection_enabled = var.deletion_protection

  point_in_time_recovery {
    enabled = true
    recovery_period_in_days = 14
  }

  attribute {
    name = "id"
    type = "S"
  }
}

resource "aws_dynamodb_table" "metadata" {
  name                        = "metadata"
  billing_mode                = "PROVISIONED"
  read_capacity               = 5
  write_capacity              = 5
  hash_key                    = "id"
  deletion_protection_enabled = var.deletion_protection

  point_in_time_recovery {
    enabled = true
    recovery_period_in_days = 14
  }

  attribute {
    name = "id"
    type = "S"
  }
}
