terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.region
}

# ── Networking (default VPC) ───────────────────────────────────────────────────

data "aws_vpc" "default" {
  default = true
}

data "aws_subnets" "default" {
  filter {
    name   = "vpc-id"
    values = [data.aws_vpc.default.id]
  }
}

data "aws_security_group" "default" {
  name   = "default"
  vpc_id = data.aws_vpc.default.id
}

# ── S3 ────────────────────────────────────────────────────────────────────────

resource "aws_s3_bucket" "pool" {
  bucket        = var.bucket_name
  force_destroy = true
}

resource "aws_s3_bucket_lifecycle_configuration" "pool" {
  bucket = aws_s3_bucket.pool.id

  rule {
    id     = "expire-bins"
    status = "Enabled"
    filter { prefix = "pool/bins/" }
    expiration { days = 7 }
  }
}

# ── ECR ───────────────────────────────────────────────────────────────────────

resource "aws_ecr_repository" "pool" {
  name                 = "tournament-pool"
  image_tag_mutability = "MUTABLE"
}

resource "aws_ecr_lifecycle_policy" "pool" {
  repository = aws_ecr_repository.pool.name
  policy = jsonencode({
    rules = [{
      rulePriority = 1
      description  = "Keep last 3 images"
      selection    = { tagStatus = "any", countType = "imageCountMoreThan", countNumber = 3 }
      action       = { type = "expire" }
    }]
  })
}

# ── IAM: Batch service role ────────────────────────────────────────────────────

resource "aws_iam_role" "batch_service" {
  name = "tournament-pool-batch-service"
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect    = "Allow"
      Principal = { Service = "batch.amazonaws.com" }
      Action    = "sts:AssumeRole"
    }]
  })
}

resource "aws_iam_role_policy_attachment" "batch_service" {
  role       = aws_iam_role.batch_service.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AWSBatchServiceRole"
}

# ── IAM: EC2 instance role (for ECS agent on Batch nodes) ─────────────────────

resource "aws_iam_role" "batch_instance" {
  name = "tournament-pool-batch-instance"
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect    = "Allow"
      Principal = { Service = "ec2.amazonaws.com" }
      Action    = "sts:AssumeRole"
    }]
  })
}

resource "aws_iam_role_policy_attachment" "batch_instance_ecs" {
  role       = aws_iam_role.batch_instance.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AmazonEC2ContainerServiceforEC2Role"
}

resource "aws_iam_instance_profile" "batch_instance" {
  name = "tournament-pool-batch-instance"
  role = aws_iam_role.batch_instance.name
}

# ── IAM: Job role (container's S3 access) ─────────────────────────────────────

resource "aws_iam_role" "batch_job" {
  name = "tournament-pool-batch-job"
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect    = "Allow"
      Principal = { Service = "ecs-tasks.amazonaws.com" }
      Action    = "sts:AssumeRole"
    }]
  })
}

resource "aws_iam_role_policy" "batch_job_s3" {
  name = "s3-pool-access"
  role = aws_iam_role.batch_job.id
  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect   = "Allow"
      Action   = ["s3:GetObject", "s3:PutObject", "s3:ListBucket"]
      Resource = [aws_s3_bucket.pool.arn, "${aws_s3_bucket.pool.arn}/*"]
    }]
  })
}

# ── IAM: Spot fleet role ───────────────────────────────────────────────────────

resource "aws_iam_role" "spot_fleet" {
  name = "tournament-pool-spot-fleet"
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Effect    = "Allow"
      Principal = { Service = "spotfleet.amazonaws.com" }
      Action    = "sts:AssumeRole"
    }]
  })
}

resource "aws_iam_role_policy_attachment" "spot_fleet" {
  role       = aws_iam_role.spot_fleet.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AmazonEC2SpotFleetTaggingRole"
}

# ── Batch compute environment ─────────────────────────────────────────────────
# 4096 max vCPUs = 4096 x 1-vCPU jobs running concurrently.
# Raise this limit via AWS Support if your account default is lower.

resource "aws_batch_compute_environment" "pool" {
  compute_environment_name = "tournament-pool"
  type                     = "MANAGED"
  service_role             = aws_iam_role.batch_service.arn

  compute_resources {
    type                = "SPOT"
    bid_percentage      = 60
    min_vcpus           = 0
    max_vcpus           = 4096
    instance_type       = ["c6a"]
    subnets             = data.aws_subnets.default.ids
    security_group_ids  = [data.aws_security_group.default.id]
    instance_role       = aws_iam_instance_profile.batch_instance.arn
    spot_iam_fleet_role = aws_iam_role.spot_fleet.arn
  }
}

# ── Batch job queue ────────────────────────────────────────────────────────────

resource "aws_batch_job_queue" "pool" {
  name     = "tournament-pool-queue"
  state    = "ENABLED"
  priority = 1

  compute_environment_order {
    order               = 1
    compute_environment = aws_batch_compute_environment.pool.arn
  }
}

# ── Batch job definition ───────────────────────────────────────────────────────

resource "aws_batch_job_definition" "worker" {
  name = "tournament-pool-worker"
  type = "container"

  container_properties = jsonencode({
    image      = "${aws_ecr_repository.pool.repository_url}:latest"
    jobRoleArn = aws_iam_role.batch_job.arn
    resourceRequirements = [
      { type = "VCPU",   value = "1"   },
      { type = "MEMORY", value = "512" }
    ]
    logConfiguration = { logDriver = "awslogs" }
  })
}
