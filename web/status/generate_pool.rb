#!/usr/bin/env ruby
# frozen_string_literal: true

# Generates {year}/pool.html by replaying tournament results through each
# checkpoint boundary, building a multi-snapshot HTML report.
#
# Usage: generate_pool.rb -y YEAR [--pool BINARY]
#
# Reads the final results.txt for YEAR from git history and replays game
# results through each auto-checkpoint boundary (pre-tournament, end of each
# day, end of each round). Runs poss in parallel for checkpoints with <= 31
# games remaining. Auto-checkpointing is handled by generate.rb — no
# --checkpoint flag needed here.

require 'date'
require 'fileutils'
require 'rbconfig'
require 'optparse'

PROJECT_ROOT = File.expand_path('../..', __dir__)
POOL_BIN     = File.join(PROJECT_ROOT, 'pool')
GENERATE_RB  = File.join(__dir__, 'generate.rb')
RUBY         = RbConfig.ruby

TOTAL_GAMES = 63

# Checkpoint boundaries — must match CHECKPOINT_GAMES in generate.rb.
CHECKPOINT_GAMES = [0, 16, 32, 40, 48, 56, 60, 62, 63].freeze

CHECKPOINT_LABELS = {
  0  => 'Pre-tournament',
  16 => 'Day 1',
  32 => 'Day 2',
  40 => 'Day 3',
  48 => 'Day 4',
  56 => 'Sweet Sixteen',
  60 => 'Elite Eight',
  62 => 'Final Four',
  63 => 'Final',
}.freeze

POSS_PROCS = 16

# Run poss starting from the Day 2 checkpoint (32 games played, 31 remaining).
# Day 1 has 47 games remaining (2^47 outcomes) — far too many to enumerate.
POSS_MIN_GAMES_PLAYED = 32

# Day offsets from tournament start (Day 1 = Thursday) for each checkpoint.
# Day 1: Thu, Day 2: Fri, Day 3: Sat, Day 4: Sun,
# Sweet Sixteen ends Fri (+8), Elite Eight ends Sun (+10),
# Final Four is Sat (+16), Championship is Mon (+18).
CHECKPOINT_DAY_OFFSETS = {
  0  => -1,
  16 =>  0,
  32 =>  1,
  40 =>  2,
  48 =>  3,
  56 =>  8,
  60 => 10,
  62 => 16,
  63 => 18,
}.freeze

# ---------------------------------------------------------------------------
# Option parsing
# ---------------------------------------------------------------------------
options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} -y YEAR [--pool BINARY]"
  opts.on('-y YEAR', 'Year directory to process (e.g. 2025)') { |y| options[:year] = y }
  opts.on('--pool BIN', 'Path to pool binary (default: ./pool)') { |b| options[:binary] = b }
end.parse!

abort "Error: -y YEAR is required\nUsage: #{$0} -y YEAR [--pool BINARY]" unless options[:year]

year     = options[:year]
pool_dir = File.join(PROJECT_ROOT, year)
binary   = options[:binary] || POOL_BIN

abort "Error: pool directory not found: #{pool_dir}"    unless Dir.exist?(pool_dir)
abort "Error: pool binary not found at #{binary}"       unless File.exist?(binary)
abort "Error: generate.rb not found at #{GENERATE_RB}" unless File.exist?(GENERATE_RB)

results_file = File.join(pool_dir, 'results.txt')
output_html  = File.join(pool_dir, 'pool.html')

# ---------------------------------------------------------------------------
# Find the commit with the final results for this year
# ---------------------------------------------------------------------------
final_commit = `git -C #{PROJECT_ROOT} log --oneline -- #{year}/results.txt 2>/dev/null`.lines.first&.split&.first
abort "Error: no git commits found touching #{year}/results.txt" if final_commit.to_s.empty?
puts "Using final commit: #{final_commit}"

# ---------------------------------------------------------------------------
# Parse final results.txt from git into sections
# ---------------------------------------------------------------------------
final_content = `git -C #{PROJECT_ROOT} show #{final_commit}:#{year}/results.txt`
abort "Error: could not read #{year}/results.txt from commit #{final_commit}" if final_content.empty?

all_sections = []
lines = final_content.lines.map(&:chomp)
i = 0
while i < lines.length
  if lines[i].start_with?('#')
    header   = lines[i]
    results  = []
    tiebreak = nil
    i += 1
    while i < lines.length && !lines[i].start_with?('#')
      if lines[i].match?(/^\d+$/)
        tiebreak = lines[i]
      elsif !lines[i].empty?
        results << lines[i]
      end
      i += 1
    end
    all_sections << { header: header, results: results, tiebreak: tiebreak }
  else
    i += 1
  end
end

# A leading section with no game results is a file title (e.g. "# 2025 Tournament Results").
if all_sections.first && all_sections.first[:results].empty?
  file_title = all_sections.shift[:header]
else
  file_title = nil
end

sections         = all_sections
total_game_lines = sections.sum { |s| s[:results].length }

puts "Parsed #{sections.length} sections from commit #{final_commit} (#{total_game_lines} total game lines):"
sections.each_with_index { |s, idx| puts "  #{idx}: #{s[:header]} (#{s[:results].length} games)" }
puts

abort "Error: expected #{TOTAL_GAMES} total game lines, found #{total_game_lines}" unless total_game_lines == TOTAL_GAMES

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

# Find the date of the first commit that added game results to results.txt.
# Returns a Date, or nil if git history can't be found.
def find_tournament_start(project_root, year)
  log_lines = `git -C #{project_root} log --reverse --format="%H %as" -- #{year}/results.txt 2>/dev/null`.lines
  log_lines.each do |line|
    sha, date_str = line.split
    content = `git -C #{project_root} show #{sha}:#{year}/results.txt 2>/dev/null`
    game_count = content.lines.count { |l| !l.start_with?('#') && !l.match?(/^\d+$/) && !l.strip.empty? }
    return Date.parse(date_str) if game_count > 0
  end
  nil
end

# Return a date label for a checkpoint given the tournament start date.
def checkpoint_date_label(start_date, games_played)
  return nil unless start_date
  offset = CHECKPOINT_DAY_OFFSETS[games_played]
  return nil unless offset
  (start_date + offset).strftime('%B %-d, %Y')
end

# Build a results.txt body containing exactly game_count game lines,
# preserving all section headers. Sections may be split mid-way.
def build_results(file_title, sections, game_count)
  parts     = file_title ? [file_title] : []
  remaining = game_count
  sections.each do |s|
    parts << s[:header]
    if remaining >= s[:results].length
      parts.concat(s[:results])
      parts << s[:tiebreak] if s[:tiebreak]
      remaining -= s[:results].length
    elsif remaining > 0
      parts.concat(s[:results].first(remaining))
      remaining = 0
    end
  end
  parts.join("\n") + "\n"
end

def remove_poss_bins(pool_dir)
  Dir.glob(File.join(pool_dir, 'poss_*.bin')).each do |f|
    FileUtils.rm(f)
    puts "  Removed bin: #{File.basename(f)}"
  end
end

def run_parallel_poss(binary, pool_dir, games_remaining)
  procs = games_remaining <= 3 ? 1 : POSS_PROCS
  puts "  Running poss (#{procs} parallel processes) ..."
  pids = (0...procs).map do |p|
    spawn(binary, '-d', pool_dir, '-b', p.to_s, '-n', procs.to_s, '-p', '-f', 'bin', 'poss',
          out: "/tmp/poss_#{p}.log", err: "/tmp/poss_#{p}.log")
  end
  failed = []
  pids.each do |pid|
    Process.wait(pid)
    failed << pid unless $?.success?
  end
  abort "  poss failed (#{failed.length} processes); see /tmp/poss_N.log" if failed.any?
end

def run_checkpoint(label:, pool_dir:, results_content:, run_poss:, games_remaining:,
                   binary:, results_file:, output_html:, timestamp: nil)
  puts "=== #{label} ==="
  File.write(results_file, results_content)

  game_lines = results_content.lines.reject { |l| l.start_with?('#') || l.strip.empty? }
  puts "  results.txt written (#{game_lines.length} game lines)"

  remove_poss_bins(pool_dir)
  run_parallel_poss(binary, pool_dir, games_remaining) if run_poss

  puts "  Running generate.rb ..."
  cmd = [RUBY, GENERATE_RB, '-d', pool_dir, '-o', output_html]
  cmd += ['--timestamp', timestamp] if timestamp
  ok = system(*cmd)
  abort "  generate.rb failed for #{label}" unless ok

  remove_poss_bins(pool_dir) if run_poss
  puts
end

# ---------------------------------------------------------------------------
# Main — save/restore results.txt, clear snapshots, replay checkpoints
# ---------------------------------------------------------------------------
snapshots_file   = File.join(pool_dir, 'web', 'snapshots.json')
original_results = File.exist?(results_file) ? File.read(results_file) : nil

if File.exist?(snapshots_file)
  FileUtils.rm(snapshots_file)
  puts "Cleared existing snapshots.json\n\n"
end

tournament_start = find_tournament_start(PROJECT_ROOT, year)
puts tournament_start ? "Tournament start date: #{tournament_start.strftime('%B %-d, %Y')}\n\n" \
                      : "Warning: could not determine tournament start date from git history\n\n"

begin
  CHECKPOINT_GAMES.each do |games_played|
    games_remaining = TOTAL_GAMES - games_played
    run_checkpoint(
      label:           CHECKPOINT_LABELS[games_played],
      pool_dir:        pool_dir,
      results_content: build_results(file_title, sections, games_played),
      run_poss:        games_played >= POSS_MIN_GAMES_PLAYED && games_remaining > 0,
      games_remaining: games_remaining,
      binary:          binary,
      results_file:    results_file,
      output_html:     output_html,
      timestamp:       checkpoint_date_label(tournament_start, games_played),
    )
  end
ensure
  if original_results
    File.write(results_file, original_results)
    puts "Restored original results.txt"
  end
end

puts "Done! Generated #{output_html}"
