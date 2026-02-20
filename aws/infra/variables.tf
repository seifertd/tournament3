variable "region" {
  description = "AWS region"
  default     = "us-east-1"
}

variable "bucket_name" {
  description = "S3 bucket for pool data and bin files"
  type        = string
}
