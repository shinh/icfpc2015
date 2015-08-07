#!/usr/bin/env ruby

require 'json'
require 'fileutils'

FileUtils.mkdir_p 'logs'

ans = []
24.times{|i|
  prob = "problems/problem_#{i}.json"
  log = "logs/#{i}.log"
  STDERR.print "#{prob} "
  start_time = Time.now
  a = JSON.load(`./play_icfp2015 -f #{prob} 2> #{log}`)
  STDERR.puts "#{Time.now - start_time} #{`tail -1 #{log}`}"
  ans += a
}

File.open('out.json', 'w') do |of|
  of.print JSON.dump(ans)
end
