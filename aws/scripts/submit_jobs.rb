#!/usr/bin/env ruby
# frozen_string_literal: true

require 'aws-sdk-s3'
require 'aws-sdk-batch'
require 'optparse'
require 'pathname'

NUM_BATCHES = 4096

options = {
  region:         'us-east-1',
  s3_prefix:      'pool',
  job_queue:      'tournament-pool-queue',
  job_definition: 'tournament-pool-worker'
}

OptionParser.new do |opts|
  opts.banner = "Usage: submit_jobs.rb -d POOL_DIR -b BUCKET [options]"
  opts.on('-d DIR',        'Local pool directory to upload')         { |v| options[:pool_dir]       = v }
  opts.on('-b BUCKET',     'S3 bucket name')                        { |v| options[:bucket]         = v }
  opts.on('--prefix P',    'S3 key prefix (default: pool)')         { |v| options[:s3_prefix]      = v }
  opts.on('--queue Q',     'Batch job queue name')                  { |v| options[:job_queue]      = v }
  opts.on('--job-def D',   'Batch job definition name')             { |v| options[:job_definition] = v }
  opts.on('--region R',    'AWS region (default: us-east-1)')       { |v| options[:region]         = v }
end.parse!

abort "Pool directory required (-d)" unless options[:pool_dir]
abort "S3 bucket required (-b)"      unless options[:bucket]

pool_dir = Pathname.new(options[:pool_dir]).expand_path
abort "Pool directory not found: #{pool_dir}" unless pool_dir.directory?

s3    = Aws::S3::Client.new(region: options[:region])
batch = Aws::Batch::Client.new(region: options[:region])

# Upload pool data to S3
data_prefix = "#{options[:s3_prefix]}/data"
puts "Uploading pool data to s3://#{options[:bucket]}/#{data_prefix}/ ..."

pool_dir.find do |path|
  next if path.directory?
  key = "#{data_prefix}/#{path.relative_path_from(pool_dir)}"
  print "  #{key}\n"
  s3.put_object(bucket: options[:bucket], key: key, body: path.binread)
end

# Submit Batch array job
job_name = "tournament-pool-poss-#{Time.now.strftime('%Y%m%d-%H%M%S')}"
puts "\nSubmitting #{NUM_BATCHES}-job Batch array as '#{job_name}'..."

resp = batch.submit_job(
  job_name:         job_name,
  job_queue:        options[:job_queue],
  job_definition:   options[:job_definition],
  array_properties: { size: NUM_BATCHES },
  container_overrides: {
    environment: [
      { name: 'POOL_S3_BUCKET',    value: options[:bucket] },
      { name: 'POOL_S3_PREFIX',    value: options[:s3_prefix] },
      { name: 'POOL_NUM_BATCHES',  value: NUM_BATCHES.to_s }
    ]
  }
)

job_id = resp.job_id
puts "Submitted job ID: #{job_id}"
puts
puts "When complete, collect results with:"
puts "  ruby aws/scripts/aggregate.rb -b #{options[:bucket]} -d #{pool_dir} --prefix #{options[:s3_prefix]}"
