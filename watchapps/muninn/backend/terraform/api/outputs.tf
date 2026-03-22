output "domain_name_url" {
  value       = aws_apigatewayv2_domain_name.domain_name.domain_name
  description = "API custom domain URL"
}

output "api_lambda_arn" {
  description = "ARN of the Lambda function"
  value = aws_lambda_function.api_lambda.arn
}

output "api_lambda_name" {
  description = "Name of the Lambda function"
  value = aws_lambda_function.api_lambda.function_name
}
