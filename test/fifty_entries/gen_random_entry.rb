#!/usr/bin/env ruby

teams = ARGF.readlines().map{|l| l.chomp.split(",") }

if teams.size != 64
  raise "Did not read enough teams: #{teams.size}"
end

games = 32
6.times do |round|
  puts "# ROUND: #{round+1}"
  winners = []
  teams.each_slice(2) do |teams|
    winners << teams.sample
  end
  if winners.size != games
    raise "Did not generate enough winners for round #{round+1}: #{winners.size} != #{games}"
  end
  winners.each do |t|
    puts t.last
  end
  teams = winners
  games = games / 2
end
puts 100 + rand(50)
