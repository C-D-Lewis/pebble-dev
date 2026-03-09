resource "null_resource" "build_lambda" {
  triggers = {
    always_run = timestamp()
  }

  provisioner "local-exec" {
    command = "cd ${path.module}/../../lambda && npm run build"
  }
}

data "archive_file" "lambda_zip" {
  type        = "zip"
  source_file = "${path.module}/../../lambda/dist/index.js"
  output_path = "function.zip"

  depends_on = [null_resource.build_lambda]
}

resource "aws_lambda_function" "api_lambda" {
  filename         = data.archive_file.lambda_zip.output_path
  function_name    = "${var.project}-api-lambda"
  role             = aws_iam_role.lambda_exec.arn
  handler          = "index.handler"
  runtime          = "nodejs24.x"
  source_code_hash = data.archive_file.lambda_zip.output_base64sha256
  memory_size      = 256
}

resource "null_resource" "cleanup_lambda_zip" {
  provisioner "local-exec" {
    command = "rm -f ${data.archive_file.lambda_zip.output_path}"
  }

  triggers = {
    always_run = timestamp()
  }

  depends_on = [aws_lambda_function.api_lambda]
}
