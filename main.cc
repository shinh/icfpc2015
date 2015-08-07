#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "problem.h"

class Lcg {
 public:
  explicit Lcg(int seed) {
    v_ = seed;
  }

  uint32_t GetNext() {
    uint32_t r = (v_ >> 16) & 0x7fff;
    v_ = v_ * 1103515245 + 12345;
    return r;
  }

  static void Test() {
    Lcg lcg(17);
    assert(lcg.GetNext() == 0);
    assert(lcg.GetNext() == 24107);
    assert(lcg.GetNext() == 16552);
    assert(lcg.GetNext() == 12125);
    assert(lcg.GetNext() == 9427);
    assert(lcg.GetNext() == 13152);
    assert(lcg.GetNext() == 21440);
    assert(lcg.GetNext() == 3383);
    assert(lcg.GetNext() == 6873);
    assert(lcg.GetNext() == 16117);
  }

 private:
  uint32_t v_;
};

class Unit {
 public:
  Unit(const vector<Pos>& members, Pos pivot)
      : members_(members), pivot_(pivot) {
  }

 private:
  vector<Pos> members_;
  Pos pivot_;
};

class Board {
 public:
  explicit Board(int width, int height, const vector<Pos>& filled) {
    W = width;
    H = height;
    b_.resize(W * H);
    for (Pos p : filled) {
      Fill(p);
    }
  }

  void Fill(Pos p) {
    assert(p.x >= 0);
    assert(p.x < W);
    assert(p.y >= 0);
    assert(p.y < H);
    size_t i = p.y * W + p.x;
    assert(i < b_.size());
    b_[i] = true;
  }

 private:
  int W, H;
  vector<int> b_;
};

class Game {
 public:
  Game(const Problem& problem, int seed)
      : lcg_(seed) {
    H = problem.height;
    W = problem.width;
    id_ = problem.id;
    source_length_ = problem.source_length;
    for (const auto& u : problem.units) {
      units_.push_back(Unit(u.first, u.second));
    }
  }

 private:
  int H, W;
  int id_;
  int source_length_;
  Lcg lcg_;
  vector<Unit> units_;
};

int main(int argc, char* argv[]) {
  Lcg::Test();

  const char* filename = "problem_0.json";
  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if (!strcmp(arg, "-f")) {
      filename = argv[++i];
    }
    // TODO: Implement the rest.
  }

  Problem problem(filename);
  for (int seed : problem.source_seeds) {
    Game game(problem, seed);
  }
}
