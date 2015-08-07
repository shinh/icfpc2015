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
      return true
    else
      return false
    end
  end

  def fix(board)
    @members.each do |x, y|
      raise if board[y][x]
      board[y][x] = true
    end
    # TODO: Clear a row.
  end

  def in?(qx, qy)
    @members.each do |x, y|
      if x == qx && y == qy
        return true
      end
    end
    return false
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

board_info = JSON.load(File.read(ARGV[0]))
height = board_info['height']
width = board_info['width']
id = board_info['id']
filled = board_info['filled']
source_length = board_info['sourceLength']
source_seeds = board_info['sourceSeeds']
units = board_info['units']

source_seeds.each_with_index do |seed, game_index|
  board = Array.new(height){[false] * width}
  filled.each do |x, y|
    board[y][x] = true
  end

  lcg = Lcg.new(seed)
  cur_unit = nil
  spawned_cnt = 0
  frame = -1
  turn = -1

  # game loop
  while true
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
      min_x = unit['members'].min{|m|m['x']}['x']
      max_x = unit['members'].max{|m|m['x']}['x']
      size_x = max_x - min_x
      base_x = (width - size_x) / 2
      cur_unit = Unit.new(unit, base_x)

      # TODO: Check if it can appear.
    end

    # TODO: Get them from stdin or somewhere.
    cmd = :move
    arg = :SW
    case cmd
    when :move
      if !cur_unit.move(arg, board)
        cur_unit.fix(board)
        cur_unit = nil
      end

    when :turn
      raise "TODO"
    else
      raise "#{cmd}"
    end

    puts "game ##{game_index} turn #{turn} @#{frame}"
    board.each_with_index do |row, y|
      if y % 2 != 0
        print ' '
      end
      row.each_with_index do |col, x|
        if cur_unit && cur_unit.in?(x, y)
          print 'x'
        elsif col
          print 'X'
        else
          print ' '
        end
        print ' '
      end
      puts
    end

  end

end
