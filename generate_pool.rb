#!/usr/bin/env ruby
# frozen_string_literal: true

# Generates {year}/pool.html by replaying tournament results through each
# round checkpoint, building a multi-snapshot HTML report.
#
# Usage: generate_pool.rb -y YEAR [--pool BINARY]
#
# Reads the final results.txt for YEAR from git history, groups sections
# into rounds by game count, and checkpoints after each round. Runs poss
# in 16 parallel processes for any round with <= 31 games remaining.

require 'fileutils'
require 'rbconfig'
require 'optparse'

ROOT_DIR    = __dir__
POOL_BIN    = File.join(ROOT_DIR, 'pool')
GENERATE_RB = File.join(ROOT_DIR, 'web', 'status', 'generate.rb')
RUBY        = RbConfig.ruby

ROUND_SIZES  = [32, 16, 8, 4, 2, 1].freeze
ROUND_LABELS = ['Round 1', 'Round 2', 'Sweet Sixteen', 'Elite Eight', 'Final Four', 'Championship'].freeze
TOTAL_GAMES  = ROUND_SIZES.sum  # 63

POSS_PROCS = 16

# Run parallel poss when this many games remain. 31 = after Round 1 (2^31
# outcomes, split across 16 processes = 2^27 each).
POSS_THRESHOLD = 31

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
pool_dir = File.join(ROOT_DIR, year)
binary   = options[:binary] || POOL_BIN

abort "Error: pool directory not found: #{pool_dir}"       unless Dir.exist?(pool_dir)
abort "Error: pool binary not found at #{binary}"          unless File.exist?(binary)
abort "Error: generate.rb not found at #{GENERATE_RB}"    unless File.exist?(GENERATE_RB)

results_file = File.join(pool_dir, 'results.txt')
output_html  = File.join(pool_dir, 'pool.html')

# ---------------------------------------------------------------------------
# Find the commit with the final results for this year
# ---------------------------------------------------------------------------
final_commit = `git log --oneline -- #{year}/results.txt 2>/dev/null`.lines.first&.split&.first
abort "Error: no git commits found touching #{year}/results.txt" if final_commit.to_s.empty?
puts "Using final commit: #{final_commit}"

# ---------------------------------------------------------------------------
# Parse final results.txt from git into sections
# ---------------------------------------------------------------------------
final_content = `git show #{final_commit}:#{year}/results.txt`
abort "Error: could not read #{year}/results.txt from commit #{final_commit}" if final_content.empty?

all_sections = []
lines = final_content.lines.map(&:chomp)
i = 0
while i < lines.length
  if lines[i].start_with?('#')
    header  = lines[i]
    results = []
    i += 1
    tiebreak = nil
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
# 2023 has no title — its first section is "# Round 1" with actual results.
if all_sections.first && all_sections.first[:results].empty?
  file_title = all_sections.shift[:header]
else
  file_title = nil
end

sections   = all_sections
total_game_lines = sections.sum { |s| s[:results].length }

puts "Parsed #{sections.length} sections from commit #{final_commit} (#{total_game_lines} total game lines):"
sections.each_with_index { |s, idx| puts "  #{idx}: #{s[:header]} (#{s[:results].length} games)" }
puts

abort "Error: expected #{TOTAL_GAMES} total game lines, found #{total_game_lines}" unless total_game_lines == TOTAL_GAMES

# ---------------------------------------------------------------------------
# Group sections into rounds by accumulating game counts
# ---------------------------------------------------------------------------
rounds = []
section_idx = 0
ROUND_SIZES.each_with_index do |target, round_idx|
  group       = []
  accumulated = 0
  while section_idx < sections.length && accumulated < target
    group       << section_idx
    accumulated += sections[section_idx][:results].length
    section_idx += 1
  end
  unless accumulated == target
    abort "Error: could not group #{target} games for #{ROUND_LABELS[round_idx]} " \
          "(accumulated #{accumulated} in sections #{group.inspect})"
  end
  rounds << group
end

puts "Round groupings:"
rounds.each_with_index do |group, idx|
  game_count = group.sum { |si| sections[si][:results].length }
  puts "  #{ROUND_LABELS[idx]}: sections #{group.inspect} (#{game_count} games)"
end
puts

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def build_results(file_title, sections, num_filled)
  parts = file_title ? [file_title] : []
  sections.each_with_index do |s, idx|
    parts << s[:header]
    if idx < num_filled
      parts.concat(s[:results])
      parts << s[:tiebreak] if s[:tiebreak]
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
    spawn(binary, '-d', pool_dir, '-b', p.to_s, '-n', procs.to_s, '-f', 'bin', 'poss',
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
                   checkpoint:, binary:, results_file:, output_html:)
  puts "=== #{label} ==="
  File.write(results_file, results_content)

  game_lines = results_content.lines.reject { |l| l.start_with?('#') || l.strip.empty? }
  puts "  results.txt written (#{game_lines.length} game lines)"

  remove_poss_bins(pool_dir)

  run_parallel_poss(binary, pool_dir, games_remaining) if run_poss

  args = [RUBY, GENERATE_RB, '-d', pool_dir, '-o', output_html]
  args << '--checkpoint' if checkpoint
  puts "  Running generate.rb#{checkpoint ? ' --checkpoint' : ''} ..."
  ok = system(*args)
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

begin
  run_checkpoint(
    label:           'Pre-tournament',
    pool_dir:        pool_dir,
    results_content: build_results(file_title, sections, 0),
    run_poss:        false,
    games_remaining: TOTAL_GAMES,
    checkpoint:      true,
    binary:          binary,
    results_file:    results_file,
    output_html:     output_html,
  )

  sections_filled = 0
  rounds.each_with_index do |group, round_idx|
    sections_filled += group.length
    games_played    = sections[0...sections_filled].sum { |s| s[:results].length }
    games_remaining = TOTAL_GAMES - games_played
    last_round      = round_idx == rounds.length - 1

    run_checkpoint(
      label:           ROUND_LABELS[round_idx],
      pool_dir:        pool_dir,
      results_content: build_results(file_title, sections, sections_filled),
      run_poss:        games_remaining > 0 && games_remaining <= POSS_THRESHOLD,
      games_remaining: games_remaining,
      checkpoint:      !last_round,
      binary:          binary,
      results_file:    results_file,
      output_html:     output_html,
    )
  end

ensure
  if original_results
    File.write(results_file, original_results)
    puts "Restored original results.txt"
  end
end

puts "Done! Generated #{output_html}"
