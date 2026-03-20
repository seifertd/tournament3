#!/usr/bin/env ruby
# frozen_string_literal: true

require 'json'
require 'optparse'
require 'open3'
require 'time'
require 'fileutils'

POOL_BINARY = File.expand_path('../../pool', __dir__)

SEED_ORDER = [1, 16, 8, 9, 5, 12, 4, 13, 6, 11, 3, 14, 7, 10, 2, 15].freeze

# ---------------------------------------------------------------------------
# Option parsing
# ---------------------------------------------------------------------------
options = { output: 'status.html', binary: POOL_BINARY }
OptionParser.new do |opts|
  opts.banner = "Usage: #{$0} -d DIR [-o OUTPUT] [--pool BINARY] [--timestamp 'LABEL']"
  opts.on('-d DIR',            'Pool directory')                        { |d| options[:dir] = d }
  opts.on('-o FILE',           'Output HTML file (status.html)')        { |f| options[:output] = f }
  opts.on('--pool BIN',        'Path to pool binary')                   { |b| options[:binary] = b }
  opts.on('--timestamp LABEL', 'Override timestamp string in snapshot') { |t| options[:timestamp] = t }
  opts.on('--checkpoint',      'Freeze current snapshot and start a new one next run') { options[:checkpoint] = true }
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

def normalize_win_prob_entries(entries, win_key)
  entries
    .map { |e| e.merge('winChance' => e[win_key].to_f) }
    .sort_by { |e| -e['winChance'] }
end

def outcomes_label(n)
  n.to_s.reverse.gsub(/(\d{3})(?=\d)/, '\1,').reverse
end

def snapshot_label(games_played)
  case games_played
  when 0     then 'Pre-tournament'
  when 1..32 then "Round 1 — #{games_played} games"
  when 33..48 then "Round 2 — #{games_played} games"
  when 49..56 then "Sweet 16 — #{games_played} games"
  when 57..60 then "Elite 8 — #{games_played} games"
  when 61..62 then "Final Four — #{games_played} games"
  else             "Final — #{games_played} games"
  end
end

# ---------------------------------------------------------------------------
# Parse config.txt for PI mappings
# ---------------------------------------------------------------------------
pi_map = {}
config_file = File.join(dir, 'config.txt')
if File.exist?(config_file)
  File.readlines(config_file, encoding: 'utf-8').each do |line|
    (1..4).each { |i| pi_map["PI#{i}"] = $1.strip if line =~ /^PI#{i}=(.+)/ }
  end
end

# ---------------------------------------------------------------------------
# Parse teams.txt
# ---------------------------------------------------------------------------
$stderr.puts "Pool directory: #{dir}"
teams_file = File.join(dir, 'teams.txt')
abort "Error: #{teams_file} not found" unless File.exist?(teams_file)

teams       = {}
regions     = []
team_num    = 1
region_seed = 0

File.readlines(teams_file, encoding: 'utf-8').each do |line|
  line.chomp!
  next if line.start_with?('#') || line.empty?
  if line.start_with?('Region:')
    regions << line[7..].strip
    region_seed = 0
  else
    name, short, = line.split(',')
    short = short.to_s.strip
    short = pi_map[short] if pi_map.key?(short)
    teams[team_num] = {
      num:       team_num,
      name:      name.to_s.strip,
      short:     short,
      seed:      SEED_ORDER[region_seed % 16],
      region:    regions.last || "Region #{(team_num - 1) / 16 + 1}",
      regionIdx: regions.length - 1,
    }
    team_num    += 1
    region_seed += 1
  end
end

# ---------------------------------------------------------------------------
# Fetch report data
# ---------------------------------------------------------------------------
$stderr.puts 'Fetching report data:'
scores_data  = run_json(binary, dir, 'scores')
entries_data = run_json(binary, dir, 'entries')

abort 'Error: could not load scores report'  unless scores_data
abort 'Error: could not load entries report' unless entries_data

pool_info       = scores_data['pool']
games_played    = pool_info['gamesPlayed']
games_remaining = pool_info['gamesRemaining']

$stderr.puts "  Pool: #{pool_info['name']} — #{games_played} played, #{games_remaining} remaining"

if games_remaining == 0
  $stderr.puts '  Pool complete — skipping win probability report'
  mc_seed = mc_model = poss = nil
elsif games_remaining > 31
  $stderr.puts '  Round 1 in progress — running Monte Carlo (both models):'
  mc_seed  = run_json(binary, dir, 'mc', '-m', 'seed')
  mc_model = run_json(binary, dir, 'mc', '-m', 'model')
  poss     = nil
else
  mc_seed = mc_model = nil
  poss    = run_json(binary, dir, 'poss')
end

ffour = games_remaining <= 3 ? run_json(binary, dir, 'ffour') : nil

# ---------------------------------------------------------------------------
# Build win probability datasets (normalized to common shape)
# ---------------------------------------------------------------------------
win_prob = []
if poss
  win_prob << {
    label:   "Possibilities — #{outcomes_label(poss['pool']['outcomes'])} outcomes",
    entries: normalize_win_prob_entries(poss['entries'], 'winChance'),
  }
end
if mc_seed
  win_prob << {
    label:   'Monte Carlo — Seed weighted',
    entries: normalize_win_prob_entries(mc_seed['entries'], 'winPct'),
  }
end
if mc_model
  win_prob << {
    label:   'Monte Carlo — Model weighted',
    entries: normalize_win_prob_entries(mc_model['entries'], 'winPct'),
  }
end

# ---------------------------------------------------------------------------
# Assemble current snapshot
# ---------------------------------------------------------------------------
t_winners = entries_data['tournament']['winners']
timestamp = options[:timestamp] || Time.now.strftime('%B %-d, %Y at %-I:%M %p')
snapshot = {
  label:             snapshot_label(games_played),
  timestamp:         timestamp,
  pool: {
    name:             pool_info['name'],
    generatedAt:      timestamp,
    gamesPlayed:      games_played,
    gamesRemaining:   games_remaining,
    finalPointsKnown: pool_info['finalPointsKnown'],
    finalPoints:      pool_info['finalPoints'],
    champion:         pool_info['champion'],
  },
  teams:             teams.values,
  regions:           regions,
  leaderboard:       scores_data['entries'],
  entries:           entries_data['entries'],
  tournamentWinners: t_winners,
  winProb:           win_prob,
  ffour:             ffour,
}

# ---------------------------------------------------------------------------
# Load snapshot history, deduplicate, and append
# ---------------------------------------------------------------------------
snapshots_dir  = File.join(dir, 'web')
snapshots_file = File.join(snapshots_dir, 'snapshots.json')

FileUtils.mkdir_p(snapshots_dir)
snapshots = File.exist?(snapshots_file) ? JSON.parse(File.read(snapshots_file)) : []

last           = snapshots.last
same_state     = last && last['tournamentWinners'] == t_winners
was_checkpoint = last && last['checkpointed']

if same_state
  $stderr.puts "  Tournament state unchanged — updating in place (#{snapshot[:label]})"
  # Preserve checkpointed flag: a checkpoint shouldn't lose its status on a re-run
  snapshots[-1] = was_checkpoint ? snapshot.merge('checkpointed' => true) : snapshot
elsif snapshots.empty? || was_checkpoint
  $stderr.puts "  New snapshot (#{snapshot[:label]})"
  snapshots << snapshot
else
  $stderr.puts "  Updating current snapshot in place (#{snapshot[:label]})"
  snapshots[-1] = snapshot
end

if options[:checkpoint]
  $stderr.puts "  Checkpointing — next run will start a new snapshot"
  snapshots[-1] = snapshots[-1].merge('checkpointed' => true)
end

File.write(snapshots_file, JSON.generate(snapshots))
$stderr.puts "  #{snapshots.length} snapshot(s) in history"

# ---------------------------------------------------------------------------
# Inject into template and write output
# ---------------------------------------------------------------------------
template_path = File.join(__dir__, 'app.html')
abort "Error: template not found at #{template_path}" unless File.exist?(template_path)

$stderr.print 'Rendering template ... '
output = File.read(template_path).sub('{{POOL_DATA}}', JSON.generate(snapshots))
File.write(options[:output], output)
$stderr.puts 'done'
puts "Generated #{options[:output]}"
