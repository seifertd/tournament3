require 'erb'
require 'cgi'

class Region
  attr_accessor :name, :games
  def initialize(name)
    self.name = name
    self.games = []
  end
end

class Game
  attr_accessor :team1, :team2
  def add_team(team)
    if self.team1.nil?
      self.team1 = team
    else
      self.team2 = team
    end
  end
end

class Team
  attr_accessor :seed, :short, :name
  def initialize(seed, short, name)
    self.seed = seed
    self.short = short
    self.name = name
  end
end

seeds = [1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15]

regions = []
seed_idx = 0

teams_file = ARGV[0]
output_file = ARGV[1]
svg_logo_file = ARGV[2]
if teams_file.nil? || output_file.nil? || svg_logo_file.nil?
  puts "Usage: ruby #{$0} <teams.txt> <output.html> <svg_logo.svg>"
  exit 1
end

teams = File.readlines(teams_file).each do |line|
  line.chomp!
  if line.start_with?("#")
    name = line[2..-1]
    regions << Region.new(name)
    seed_idx = 0
  else
    name, short = line.split(',')
    seed = seeds[seed_idx]
    game_idx = seed_idx / 2
    regions.last.games[game_idx] ||= Game.new
    game = regions.last.games[game_idx]
    game.add_team(Team.new(seed, short, name))
    seed_idx += 1
  end
end

svg_logo = File.read(svg_logo_file).chomp
template = File.read("#{__dir__}/bracket.html.erb")
erb = ERB.new(template, trim_mode: "%<>")
File.write(output_file, erb.result(binding))
