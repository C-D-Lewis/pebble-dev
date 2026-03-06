data "archive_file" "lambda_zip" {
  type        = "zip"
  source_file = "../lambda/index.mjs"
  output_path = "function.zip"
}

resource "aws_lambda_function" "api_lambda" {
  filename         = data.archive_file.lambda_zip.output_path
  function_name    = "${var.project}-api-lambda"
  role             = aws_iam_role.lambda_exec.arn
  handler          = "index.handler"
  runtime          = "nodejs20.x"
  source_code_hash = data.archive_file.lambda_zip.output_base64sha256
  memory_size      = 256
}
