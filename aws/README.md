# AWS Parallel Possibilities Report

Runs the `poss` report overnight after Day 1 of Round 1 using 4,096 AWS Batch
Spot jobs. At that point 47 games remain (2^47 ≈ 140 trillion outcomes), which
takes roughly 4 hours across 4,096 workers at ~2.3M DFS nodes/sec each.

**Estimated cost: ~$280 on c6a Spot instances.**

> **Warning:** This implementation is untested. The cost estimate is based on
> back-of-the-envelope math and actual AWS Spot pricing at time of writing — your
> mileage may vary. Verify the Terraform, IAM roles, and job definitions against
> current AWS Batch documentation before running.

For later rounds (≤36 games remaining), a 12-core local machine handles it in
under an hour for free — skip this entirely and use `parallel.sh` instead.

---

## Prerequisites

- AWS CLI configured with credentials (`aws configure`)
- Terraform ≥ 1.0
- Docker
- Ruby with Bundler (`gem install bundler`)
- `pool` binary built locally (`make`)

The AWS credentials need permissions for: S3, ECR, Batch, and IAM (for
`terraform apply`). A power-user or admin role works for the one-time setup.
Day-to-day usage (submit + aggregate) only needs S3 and Batch access.

---

## One-Time Setup

### 1. Provision infrastructure

```console
$ cd aws/infra
$ terraform init
$ terraform apply -var="bucket_name=my-tournament-pool"
```

Note the outputs — you'll need the ECR repository URL.

> **vCPU limit:** AWS accounts default to 256–1,024 vCPUs for Batch. With 4,096
> jobs each using 1 vCPU you will need a limit increase. Request it via the
> Service Quotas console under *AWS Batch → EC2 vCPUs* before tournament day.

### 2. Build and push the Docker image

```console
$ cd /path/to/tournament3

$ aws ecr get-login-password --region us-east-1 \
    | docker login --username AWS --password-stdin <ecr-repository-url>

$ docker build -f aws/Dockerfile -t tournament-pool .
$ docker tag tournament-pool:latest <ecr-repository-url>:latest
$ docker push <ecr-repository-url>:latest
```

Rebuild and push the image any time `pool.c` or `pool.h` changes.

### 3. Install Ruby dependencies

```console
$ cd aws
$ bundle install
```

---

## Each Year's Tournament

### Night of Day 1 (after the last game finishes)

Make sure `results.txt` is up to date with all 16 Day 1 winners, then submit:

```console
$ ruby aws/scripts/submit_jobs.rb -d 2025/ -b my-tournament-pool
```

This uploads your pool directory to S3 and submits a 4,096-job Batch array. It
prints the job ID and the exact `aggregate.rb` command to run in the morning,
then exits. Jobs run overnight unattended.

Monitor progress in the AWS Batch console or:

```console
$ aws batch describe-jobs --jobs <job-id> \
    --query 'jobs[0].{status:status,succeeded:arrayProperties.statusSummary.SUCCEEDED}'
```

### Morning (after all jobs show SUCCEEDED)

```console
$ ruby aws/scripts/aggregate.rb -b my-tournament-pool -d 2025/
```

This downloads all 4,096 `.bin` files from S3 in parallel (32 threads) into a
temp directory, then runs `./pool -r poss` locally to produce the report.

---

## Reference

### `submit_jobs.rb` options

| Flag | Default | Description |
|---|---|---|
| `-d DIR` | *(required)* | Local pool directory to upload |
| `-b BUCKET` | *(required)* | S3 bucket name |
| `--prefix P` | `pool` | S3 key prefix |
| `--queue Q` | `tournament-pool-queue` | Batch job queue |
| `--job-def D` | `tournament-pool-worker` | Batch job definition |
| `--region R` | `us-east-1` | AWS region |

### `aggregate.rb` options

| Flag | Default | Description |
|---|---|---|
| `-b BUCKET` | *(required)* | S3 bucket name |
| `-d DIR` | *(required)* | Local pool directory (for teams/config/results) |
| `--prefix P` | `pool` | S3 key prefix |
| `--pool BIN` | `../../pool` | Path to pool binary |
| `--region R` | `us-east-1` | AWS region |

### S3 layout

```
s3://my-tournament-pool/
  pool/
    data/          ← uploaded by submit_jobs.rb
      config.txt
      teams.txt
      results.txt
      entries/
        *.txt
    bins/          ← written by workers, read by aggregate.rb
      poss_0_of_4096.bin
      poss_1_of_4096.bin
      ...
```

Bin files expire automatically after 7 days (configured in Terraform).
