output "s3_bucket" {
  value = aws_s3_bucket.pool.bucket
}

output "ecr_repository_url" {
  value = aws_ecr_repository.pool.repository_url
}

output "job_queue" {
  value = aws_batch_job_queue.pool.name
}

output "job_definition" {
  value = aws_batch_job_definition.worker.name
}
