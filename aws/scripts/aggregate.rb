#!/usr/bin/env ruby
# frozen_string_literal: true

require 'aws-sdk-s3'
require 'optparse'
require 'pathname'
require 'tmpdir'
require 'fileutils'
require 'thread'

NUM_BATCHES      = 4096
DOWNLOAD_THREADS = 32

options = {
  region:    'us-east-1',
  s3_prefix: 'pool',
  pool_bin:  File.expand_path('../../../pool', __dir__)
}

OptionParser.new do |opts|
  opts.banner = "Usage: aggregate.rb -b BUCKET -d POOL_DIR [options]"
  opts.on('-b BUCKET',   'S3 bucket name')                        { |v| options[:bucket]   = v }
  opts.on('-d DIR',      'Local pool directory (config/teams/results)') { |v| options[:pool_dir] = v }
  opts.on('--prefix P',  'S3 key prefix (default: pool)')         { |v| options[:s3_prefix] = v }
  opts.on('--pool BIN',  'Path to pool binary')                   { |v| options[:pool_bin]  = v }
  opts.on('--region R',  'AWS region (default: us-east-1)')       { |v| options[:region]    = v }
end.parse!

abort "S3 bucket required (-b)"      unless options[:bucket]
abort "Pool directory required (-d)" unless options[:pool_dir]
abort "pool binary not found at #{options[:pool_bin]} (build first with make, or pass --pool)" \
  unless File.executable?(options[:pool_bin])

pool_dir = Pathname.new(options[:pool_dir]).expand_path
s3       = Aws::S3::Client.new(region: options[:region])
bins_prefix = "#{options[:s3_prefix]}/bins"

Dir.mktmpdir('pool-aggregate') do |tmpdir|
  # Copy local pool data (teams, config, results, entries) into tmpdir
  FileUtils.cp_r("#{pool_dir}/.", tmpdir)

  # Parallel download of bin files from S3
  puts "Downloading #{NUM_BATCHES} bin files from s3://#{options[:bucket]}/#{bins_prefix}/ ..."

  queue     = Queue.new
  NUM_BATCHES.times { |i| queue << i }

  completed = 0
  mutex     = Mutex.new

  threads = DOWNLOAD_THREADS.times.map do
    Thread.new do
      client = Aws::S3::Client.new(region: options[:region])
      while (i = queue.pop(true) rescue nil)
        key   = "#{bins_prefix}/poss_#{i}_of_#{NUM_BATCHES}.bin"
        local = File.join(tmpdir, "poss_#{i}_of_#{NUM_BATCHES}.bin")
        client.get_object(response_target: local, bucket: options[:bucket], key: key)
        mutex.synchronize do
          completed += 1
          print "\r  #{completed}/#{NUM_BATCHES}" if (completed % 100).zero? || completed == NUM_BATCHES
        end
      end
    end
  end
  threads.each(&:join)
  puts "\n  Done."

  puts "\nRunning possibilities report..."
  exec options[:pool_bin], '-d', tmpdir, '-r', 'poss'
end
