#!/usr/bin/env ruby

require 'json'
require 'fileutils'

system("make")

FileUtils.mkdir_p 'logs'

ans = []
24.times{|i|
  prob = "problems/problem_#{i}.json"
  logfile = "logs/#{i}.log"
  STDERR.print "#{prob} "
  start_time = Time.now
  a = JSON.load(`./play_icfp2015 -f #{prob} 2> #{logfile}`)

  log = File.read(logfile)
  score = 0
  results = []
  log.scan(/^turn=.* score=(\d+)/) do
    results << $&
    score += $1.to_i
  end

  #STDERR.puts "#{Time.now - start_time} score=#{score/results.size} #{results}"
  STDERR.puts "#{Time.now - start_time} score=#{score/results.size}"
  ans += a
}

File.open('out.json', 'w') do |of|
  of.print JSON.dump(ans)
end
