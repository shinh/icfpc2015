#!/usr/bin/env ruby

require 'json'
require 'fileutils'

FileUtils.mkdir_p 'logs'

ans = []
24.times{|i|
  prob = "problems/problem_#{i}.json"
  STDERR.print "#{prob} "
  start_time = Time.now
  a = JSON.load(`./play_icfp2015 -f #{prob} 2> logs/#{i}.log`)
  STDERR.puts Time.now - start_time
  ans += a
}

File.open('out.json', 'w') do |of|
  of.print JSON.dump(ans)
end
