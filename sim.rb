#!/usr/bin/env ruby

require 'json'

def assert_eq(a, b)
  if a != b
    raise "#{a} vs #{b}"
  end
end

class Lcg
  def initialize(seed)
    @v = seed
  end

  def raw
    r = (@v >> 16) & 0x7fff
    @v = (@v * 1103515245 + 12345) & (2 ** 32 - 1)
    r
  end

  def pick(n)
    raw % n
  end
end

def decode_cmd(s)
  r = []
  s.each_char do |c|
    if [?p, ?', ?!, ?., ?0, ?3].include?(c)
      r << [:move, :W]
    elsif [?b, ?c, ?e, ?f, ?y, ?2].include?(c)
      r << [:move, :E]
    elsif [?a, ?g, ?h, ?i, ?j, ?4].include?(c)
      r << [:move, :SW]
    elsif [?l, ?m, ?n, ?o, ' ', ?5].include?(c)
      r << [:move, :SE]
    elsif [?d, ?q, ?r, ?v, ?z, ?1].include?(c)
      r << [:turn, :C]
    elsif [?k, ?s, ?t, ?u, ?w, ?x].include?(c)
      r << [:turn, :CC]
    else
      raise "Unknown: #{c}"
    end
  end
  r
end

class Unit
  attr_accessor :members, :pivot

  def initialize(u, base_x)
    @members = []
    u['members'].each do |pos|
      @members << [pos['x'] + base_x, pos['y']]
    end
    @pivot = [u['pivot']['x'] + base_x, u['pivot']['y']]
  end

  def moved_pos(x, y, dir)
    dx = dy = 0
    case dir
    when :E
      dx = 1
      dy = 0
    when :SE
      dx = y % 2
      dy = 1
    when :W
      dx = -1
      dy = 0
    when :SW
      dx = -1 + y % 2
      dy = 1
    else
      raise "#{arg}"
    end
    [x + dx, y + dy]
  end

  def rotated_pos(x, y, dir)
    #puts "rotated_pos (#{x}, #{y}), (#{@pivot[0]}, #{@pivot[1]})"
    xdist = Math.sqrt(3)/2
    ydist = 3.0/4.0
    geox = xdist * x + (1 + (y % 2)) * xdist / 2
    geoy = 0.5 + ydist * y
    geopx = xdist * @pivot[0] + (1 + (@pivot[1] % 2)) * xdist / 2
    geopy = 0.5 + ydist * @pivot[1]

    dx = geox - geopx
    dy = geoy - geopy
    case dir
    when :C
      theta = Math::PI / 3
    when :CC
      theta = - Math::PI / 3
    else
      raise "#{arg}"
    end

    geonx = dx * Math.cos(theta) - dy * Math.sin(theta) + geopx
    geony = dx * Math.sin(theta) + dy * Math.cos(theta) + geopy

    #puts "rotated_pos geo (#{geox}, #{geoy}), (#{geopx}, #{geopy}), (#{geonx}, #{geony})"
    ny = ((geony - 0.5) / ydist).round
    nx = ((geonx - (1 + (ny % 2)) * xdist / 2) / xdist).round
    #puts "rotated_pos after (#{nx}, #{ny})"
    [nx, ny]
  end

  def can_move(dir, board)
    @members.each do |x, y|
      nx, ny = moved_pos(x, y, dir)
      if (nx < 0 || ny < 0 || ny >= board.size || nx >= board[0].size ||
          board[ny][nx])
        return false
      end
    end
    return true
  end

  def move(dir, board)
    if can_move(dir, board)
      @members.each do |pos|
        x, y = pos
        nx, ny = moved_pos(x, y, dir)
        pos[0] = nx
        pos[1] = ny
      end
      @pivot = moved_pos(@pivot[0], @pivot[1], dir)
      return true
    else
      return false
    end
  end

  def can_turn(dir, board)
    @members.each do |x, y|
      nx, ny = rotated_pos(x, y, dir)
      if (nx < 0 || ny < 0 || ny >= board.size || nx >= board[0].size ||
          board[ny][nx])
        return false
      end
    end
    return true
  end

  def turn(dir, board)
    if can_turn(dir, board)
      @members.each do |pos|
        x, y = pos
        nx, ny = rotated_pos(x, y, dir)
        pos[0] = nx
        pos[1] = ny
      end
      return true
    else
      return false
    end
  end

  def put(board)
    @members.each do |x, y|
      raise if board[y][x]
      board[y][x] = true
    end
  end

  def fix(board, width, height, ls_old)
    put(board)

    ls = 0
    0.upto(height - 1){|i|
      line_delete = true
      0.upto(width - 1){|j|
        if board[i][j] == false then
          line_delete = false
          break
        end
      }

      if line_delete then
        ls = ls + 1
        #puts "line deleted #{i}, #{j}"
        i.downto(1){|k|
          0.upto(width - 1){|l|
            board[k][l] = board[k-1][l]
          }
        }
        0.upto(width - 1){|k| board[0][k] = false}
      end
    }

    score = @members.size + 100 * (1 + ls) * ls / 2
    puts "unit move score = #{score}"
    line_bonus = 0
    if ls_old > 1 then
      score = score + ((ls_old - 1) * score / 10).floor
    end
    puts "unit whole score = #{score}"
    
    [score, ls]
  end

  def in?(qx, qy)
    @members.each do |x, y|
      if x == qx && y == qy
        return true
      end
    end
    return false
  end

  def can_appear(board)
    @members.each do |x, y|
      return false if board[y][x]
    end
    return true
  end
end

def show_board(board, cur_unit)
  board.each_with_index do |row, y|
    if y % 2 != 0
      print ' '
    end
    print '|'
    row.each_with_index do |col, x|
      if cur_unit && cur_unit.in?(x, y)
        print 'o'
      elsif col
        print 'X'
      else
        print ' '
      end
      print '|'
    end
    puts ""
  end
end

lcg = Lcg.new(17)
assert_eq lcg.raw, 0
assert_eq lcg.raw, 24107
assert_eq lcg.raw, 16552
assert_eq lcg.raw, 12125
assert_eq lcg.raw, 9427
assert_eq lcg.raw, 13152
assert_eq lcg.raw, 21440
assert_eq lcg.raw, 3383
assert_eq lcg.raw, 6873
assert_eq lcg.raw, 16117

ls_old = 0
board_info = JSON.load(File.read(ARGV[0]))
height = board_info['height']
width = board_info['width']
id = board_info['id']
filled = board_info['filled']
source_length = board_info['sourceLength']
source_seeds = board_info['sourceSeeds']
units = board_info['units']

solution_all = JSON.load(File.read(ARGV[1]))

total_score = 0
source_seeds.each_with_index do |seed, game_index|
  solution = nil
  solution_all.each{|e|
    if e['problemId'].to_i == id && e['seed'].to_i == seed then
      solution = e['solution']
    end
  }
  puts "Given solution is #{solution[0,800]}... (len=#{solution.size})"

  board = Array.new(height){[false] * width}
  filled.each do |m|
    board[m['y']][m['x']] = true
  end

  lcg = Lcg.new(seed)
  cur_unit = nil
  spawned_cnt = 0
  frame = -1
  turn = 0
  ls_old = 0
  seen_boards = nil

  cmds = decode_cmd(solution)
  #cmds = decode_cmd('pppppppppadddddd')
  #cmds = decode_cmd('ppppppppapppplbbbbbbbappppppppppppabbbbbbbbapppppppabbbbbbbbappppppppabbbbbbbbbappppppppdbbbbbbbdppppppppdppppplbbbbbbbappppppplbbbbbbbapppppppdbbbbbbbdpppppppdppppplbbbbbbbappppppplbbbbbbbappppppabbbbbbbappppppabbbbbbbapppppppabbbbbbbdppppppppppppppplbbbbbbapppppppppplbbbbbbapppppplbbbbbbappppppdbbbbbbdppppppdpppplbbbbbbkpppppkbbbbbkpppppkpkppbbbbapppplbbbbappppplbbbbbappppplbbbbbapppppppbbbbapppaplbbbbbappppplbbbbbapppppappdpdppbbbbbappppabbbbbapppppppbbbbdpppdpdpp')

  # game loop
  while frame < cmds.size - 1
    frame += 1
    if !cur_unit
      turn += 1
      if spawned_cnt == source_length
        # TODO: What should we do, then?
        puts 'no more puyos'
        break
      end
      spawned_cnt += 1

      index = lcg.pick(units.size)
      unit = units[index]
      xs = unit['members'].map{|m|m['x']}
      min_x = xs.min
      max_x = xs.max
      size_x = max_x - min_x + 1
      base_x = (width - size_x) / 2 - min_x
      puts "hoge #{min_x}, #{max_x}, #{size_x}, #{base_x}"
      cur_unit = Unit.new(unit, base_x)

      if !cur_unit.can_appear(board) then
        puts 'Unit cannot appear to this board. Finishing game.'
        break
      end

      seen_boards = {}
      puts "game ##{game_index} turn #{turn} @#{frame} New unit pivot(#{cur_unit.pivot[0]},#{cur_unit.pivot[1]})."
      show_board(board, cur_unit)
    end

    nb = board.map{|r|r.dup}
    cur_unit.put(nb)
    if seen_boards[[cur_unit.pivot, nb]]
      raise "Error: We've seen the same board @#{seen_boards[[cur_unit.pivot, nb2]]}! @#{ARGV[0]}"
    end
    seen_boards[[cur_unit.pivot, nb]] = frame

    cmd = cmds[frame]
    if cmd
      cmd, arg = *cmd
    else
      raise "invalid cmds"
    end

    unit_score = nil
    is_decision = false
    case cmd
    when :move
      if !cur_unit.move(arg, board)
        is_decision = true
        unit_score, ls_old = cur_unit.fix(board, width, height, ls_old)
        total_score = total_score + unit_score
      else
        nb2 = board.map{|r|r.dup}
        cur_unit.put(nb2)
        if seen_boards[[cur_unit.pivot, nb2]]
          raise "Error: We've seen the same board @#{seen_boards[[cur_unit.pivot, nb2]]}! @#{ARGV[0]}"
        end
      end

    when :turn
      if !cur_unit.turn(arg, board)
        is_decision = true
        unit_score, ls_old = cur_unit.fix(board, width, height, ls_old)
        total_score = total_score + unit_score
      else
        nb2 = board.map{|r|r.dup}
        cur_unit.put(nb2)
        if seen_boards[[cur_unit.pivot, nb2]]
          raise "Error: We've seen the same board @#{seen_boards[[cur_unit.pivot, nb2]]}! @#{ARGV[0]}"
        end
      end
    else
      raise "#{cmd}"
    end

    if is_decision
      puts "game ##{game_index} turn #{turn} decision"
      show_board(board, cur_unit)
      cur_unit = nil
    else
      puts "game ##{game_index} turn #{turn} @#{frame} #{cmd} #{arg}"
      show_board(board, cur_unit)
    end
  end

  if solution.size > frame + 1
    raise "WARNING: too long solution? #{solution.size} vs #{frame}"
  end

end

puts "total_score is #{total_score}."
