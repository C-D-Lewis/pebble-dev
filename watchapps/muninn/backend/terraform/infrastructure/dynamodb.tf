resource "aws_dynamodb_table" "ids_table" {
  name                        = "ids"
  billing_mode                = "PAY_PER_REQUEST"
  hash_key                    = "id"
  deletion_protection_enabled = var.deletion_protection

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

    key_schema {
      attribute_name = "watchToken"
      key_type       = "HASH"
    }
  }
}

resource "aws_dynamodb_table" "history_table" {
  name                        = "history"
  billing_mode                = "PAY_PER_REQUEST"
  hash_key                    = "id"
  deletion_protection_enabled = var.deletion_protection

  attribute {
    name = "id"
    type = "S"
  }
}
