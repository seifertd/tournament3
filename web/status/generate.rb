#!/usr/bin/env ruby
# frozen_string_literal: true

require 'json'
require 'erb'
require 'optparse'
require 'open3'
require 'time'
require 'set'
require 'cgi'

POOL_BINARY = File.expand_path('../../pool', __dir__)

ROUND_STARTS = [0, 32, 48, 56, 60, 62].freeze
ROUND_SIZES  = [32, 16, 8, 4, 2, 1].freeze
ROUND_NAMES  = ['Round of 64', 'Round of 32', 'Sweet 16', 'Elite 8', 'Final Four', 'Championship'].freeze
SEED_ORDER   = [1, 16, 8, 9, 5, 12, 4, 13, 6, 11, 3, 14, 7, 10, 2, 15].freeze

# ---------------------------------------------------------------------------
# Option parsing
# ---------------------------------------------------------------------------
options = { output: 'status.html', binary: POOL_BINARY }
OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} -d DIR [-o OUTPUT] [--pool BINARY]"
  opts.on('-d DIR',    'Pool directory')                 { |d| options[:dir] = d }
  opts.on('-o FILE',   'Output HTML file (status.html)') { |f| options[:output] = f }
  opts.on('--pool BIN','Path to pool binary')            { |b| options[:binary] = b }
end.parse!

abort "Error: -d DIR is required\nUsage: #{$0} -d DIR [-o OUTPUT]" unless options[:dir]

dir    = File.expand_path(options[:dir])
binary = options[:binary]
abort "Error: pool binary not found at #{binary}" unless File.exist?(binary)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def run_json(binary, dir, *args)
  cmd = [binary, '-d', dir, '-f', 'json'] + args
  $stderr.print "  Running: #{args.join(' ')} ... "
  $stderr.flush
  t0 = Time.now
  out, err, status = Open3.capture3(*cmd)
  elapsed = Time.now - t0
  unless status.success?
    $stderr.puts "FAILED (#{elapsed.round(1)}s)"
    warn "  Warning: #{err.strip}"
    return nil
  end
  $stderr.puts "done (#{elapsed.round(1)}s)"
  JSON.parse(out)
rescue JSON::ParserError => e
  $stderr.puts "JSON parse error: #{e.message}"
  nil
end

def round_of_game(game)
  ROUND_STARTS.each_with_index do |start, r|
    next_start = ROUND_STARTS[r + 1] || 63
    return r if game >= start && game < next_start
  end
  nil
end

# Returns [src_game1, src_game2] for a given game (nil for R1 games)
def source_games(game)
  r = round_of_game(game)
  return nil if r == 0
  prev_start = ROUND_STARTS[r - 1]
  offset = game - ROUND_STARTS[r]
  [prev_start + 2 * offset, prev_start + 2 * offset + 1]
end

# Build set of eliminated team numbers from tournament winners array
def eliminated_teams(t_winners)
  eliminated = Set.new
  63.times do |game|
    winner = t_winners[game]
    next if winner == 0

    r = round_of_game(game)
    if r == 0
      # R1: loser is the other team in the pair (teams are 1-indexed, pairs are consecutive)
      team1 = 2 * game + 1
      team2 = 2 * game + 2
      loser = (winner == team1) ? team2 : team1
      eliminated << loser
    else
      # Later rounds: loser is the winner of one of the two source games
      src1, src2 = source_games(game)
      [src1, src2].each do |src|
        t = t_winners[src]
        eliminated << t if t > 0 && t != winner
      end
    end
  end
  eliminated
end

# ---------------------------------------------------------------------------
# Parse teams.txt
# ---------------------------------------------------------------------------
$stderr.puts "Pool directory: #{dir}"
teams_file = File.join(dir, 'teams.txt')
abort "Error: #{teams_file} not found" unless File.exist?(teams_file)

teams = {}       # 1-indexed team number => hash
regions = []
team_num = 1
region_seed_idx = 0

File.readlines(teams_file, encoding: 'utf-8').each do |line|
  line.chomp!
  next if line.start_with?('#') || line.empty?
  if line.start_with?('Region:')
    regions << line[7..].strip
    region_seed_idx = 0
  else
    name, short = line.split(',', 2)
    seed = SEED_ORDER[region_seed_idx % 16]
    teams[team_num] = {
      num:        team_num,
      name:       name.to_s.strip,
      short:      short.to_s.strip,
      seed:       seed,
      region:     regions.last || "Region #{(team_num - 1) / 16 + 1}",
      region_idx: regions.length - 1
    }
    team_num += 1
    region_seed_idx += 1
  end
end

# ---------------------------------------------------------------------------
# Fetch report data
# ---------------------------------------------------------------------------
$stderr.puts 'Fetching report data:'
scores_data  = run_json(binary, dir, 'scores')
entries_data = run_json(binary, dir, 'entries')

abort 'Error: could not load scores report' unless scores_data
abort 'Error: could not load entries report' unless entries_data

pool_name          = scores_data['pool']['name']
games_played       = scores_data['pool']['gamesPlayed']
games_remaining    = scores_data['pool']['gamesRemaining']
final_points_known = scores_data['pool']['finalPointsKnown']
final_points       = scores_data['pool']['finalPoints']
champion_short     = scores_data['pool'].fetch('champion', nil)

$stderr.puts "  Pool: #{pool_name} — #{games_played} games played, #{games_remaining} remaining"

t_winners = entries_data['tournament']['winners']  # array of 63 ints
eliminated = eliminated_teams(t_winners)

# Build a lookup: short_name => team_num for resolving champion
short_to_num = teams.transform_values { |t| t[:short] }.invert

# Use Monte Carlo while round 1 is still in progress (>31 games remaining),
# switch to the full possibilities report once day 2 is complete,
# and skip both when the pool is finished (no games remaining).
if games_remaining == 0
  $stderr.puts '  Pool complete — skipping win probability report'
  mc_seed  = nil
  mc_model = nil
  poss     = nil
elsif games_remaining > 31
  $stderr.puts '  Round 1 in progress — running Monte Carlo (both models):'
  mc_seed  = run_json(binary, dir, 'mc', '-m', 'seed')
  mc_model = run_json(binary, dir, 'mc', '-m', 'model')
  poss     = nil
else
  mc_seed  = nil
  mc_model = nil
  poss     = run_json(binary, dir, 'poss')
end

# Final four payouts: only available when <=3 games remain.
ffour = games_remaining <= 3 ? run_json(binary, dir, 'ffour') : nil

# ---------------------------------------------------------------------------
# Build derived data for the template
# ---------------------------------------------------------------------------

# Leaderboard entries (already sorted by score in scores_data)
leaderboard = scores_data['entries']

# Entry brackets (sorted by name in entries_data when pool in progress)
entry_brackets = entries_data['entries']

# For each entry bracket, annotate each pick with its status
# status: :correct, :wrong, :future, :impossible
def pick_status(game, pick_team, t_winners, eliminated)
  actual = t_winners[game]
  if actual == 0
    # Game not played yet — but check if picked team is already eliminated
    eliminated.include?(pick_team) ? :impossible : :future
  elsif pick_team == actual
    :correct
  else
    :wrong
  end
end

generated_at = Time.now.strftime('%B %-d, %Y at %-I:%M %p')

# ---------------------------------------------------------------------------
# Render template
# ---------------------------------------------------------------------------
template_path = File.join(__dir__, 'status.html.erb')
abort "Error: template not found at #{template_path}" unless File.exist?(template_path)

$stderr.print 'Rendering template ... '
erb    = ERB.new(File.read(template_path), trim_mode: '%<>')
output = erb.result(binding)
File.write(options[:output], output)
$stderr.puts 'done'
puts "Generated #{options[:output]}"
