#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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

enum Command {
  MOVE_W,
  MOVE_E,
  MOVE_SW,
  MOVE_SE,
  ROT_C,
  ROT_CC,
};

string MakeCommandStr(const vector<Command>& cmds) {
  string r;
  for (Command c : cmds) {
    switch (c) {
      case MOVE_W:
        r += 'p';
        break;
      case MOVE_E:
        r += 'b';
        break;
      case MOVE_SW:
        r += 'a';
        break;
      case MOVE_SE:
        r += 'l';
        break;
      case ROT_C:
        r += 'd';
        break;
      case ROT_CC:
        r += 'k';
        break;
    }
  }
  return r;
}

#define PF(p) p.x, p.y

struct Pos {
  int x, y;
  Pos() : x(-1), y(-1) {}
  Pos(int x0, int y0) : x(x0), y(y0) {}
  explicit Pos(pair<int, int> p) : x(p.first), y(p.second) {}

  static void CheckEq(Pos a, Pos b, const char* f, int l) {
    if (a != b) {
      fprintf(stderr, "%s:%d: (%d,%d) vs (%d,%d)\n",
              f, l, a.x, a.y, b.x, b.y);
    }
  }

  static void Test() {
#define CHECK_EQ(a, b) CheckEq(a, b, __FILE__, __LINE__)
    CHECK_EQ(Pos(0, 0).Rotate(0, 0, Pos(0, 0)), Pos(0, 0));

    CHECK_EQ(Pos(1, 0).Rotate(0, 0, Pos(0, 0)), Pos(1, 0));
    CHECK_EQ(Pos(1, 0).Rotate(0, 1, Pos(0, 0)), Pos(0, 1));
    CHECK_EQ(Pos(1, 0).Rotate(0, 2, Pos(0, 0)), Pos(-1, 1));
    CHECK_EQ(Pos(1, 0).Rotate(0, 3, Pos(0, 0)), Pos(-1, 0));
    CHECK_EQ(Pos(1, 0).Rotate(0, 4, Pos(0, 0)), Pos(-1, -1));
    CHECK_EQ(Pos(1, 0).Rotate(0, 5, Pos(0, 0)), Pos(0, -1));

    CHECK_EQ(Pos(1, 0).Rotate(1, 0, Pos(0, 0)), Pos(1, 0));
    CHECK_EQ(Pos(1, 0).Rotate(1, 1, Pos(0, 0)), Pos(1, 1));
    CHECK_EQ(Pos(1, 0).Rotate(1, 2, Pos(0, 0)), Pos(0, 1));
    CHECK_EQ(Pos(1, 0).Rotate(1, 3, Pos(0, 0)), Pos(-1, 0));
    CHECK_EQ(Pos(1, 0).Rotate(1, 4, Pos(0, 0)), Pos(0, -1));
    CHECK_EQ(Pos(1, 0).Rotate(1, 5, Pos(0, 0)), Pos(1, -1));

    CHECK_EQ(Pos(2, 0).Rotate(0, 0, Pos(1, 1)), Pos(2, 0));
    CHECK_EQ(Pos(2, 0).Rotate(0, 1, Pos(1, 1)), Pos(2, 1));
    CHECK_EQ(Pos(2, 0).Rotate(0, 2, Pos(1, 1)), Pos(2, 2));
    CHECK_EQ(Pos(2, 0).Rotate(0, 3, Pos(1, 1)), Pos(1, 2));
    CHECK_EQ(Pos(2, 0).Rotate(0, 4, Pos(1, 1)), Pos(0, 1));
    CHECK_EQ(Pos(2, 0).Rotate(0, 5, Pos(1, 1)), Pos(1, 0));
#undef CHECK_EQ
  }

  bool operator==(Pos p) const {
    return x == p.x && y == p.y;
  }
  bool operator!=(Pos p) const {
    return !operator==(p);
  }

  void operator+=(Pos p) {
    x += p.x;
    y += p.y;
  }

  double GetGeomX(int cy) const {
    return x + ((y + cy) & 1) * 0.5;
  }

  Pos MoveSW() const {
    return Pos(x - 1 + y % 2, y + 1);
  }

  Pos MoveSE() const {
    return Pos(x + y % 2, y + 1);
  }

  Pos Rotate(int cy, int r, Pos p) {
    double dy = sqrt(1-0.5*0.5);
    double pgx = p.GetGeomX(cy);
    double gx = GetGeomX(cy) - pgx;
    double gy = (y - p.y) * dy;
    double gr = M_PI * 2 * r / 6;
    double nx = gx * cos(gr) - gy * sin(gr);
    double ny = gx * sin(gr) + gy * cos(gr);
    int iy = round(ny / dy) + p.y;
    int ix = round(nx + pgx - ((iy - y + cy) & 1) * 0.5);
    //fprintf(stderr, "(%f,%f) => (%f,%f) %d,%d\n", gx, gy, nx, ny, ix, iy);
    return Pos(ix, iy);

#if 0
    int dx = x - p.x;
    int dy = y - p.y;
    int nx = 0;
    int ny = 0;
    int odd = (y + cy) & 1;
    switch (r) {
      case 0:
        return *this;

      case 1: {
      }

      case 2:
      case 3:
      case 4:
      case 5:

      default:
        assert(false);
    }
#endif
  }
};

struct Decision {
  Decision(int x0, int y0, int r0)
      : x(x0), y(y0), r(r0) {
  }
  Decision(Pos p, int r0)
      : x(p.x), y(p.y), r(r0) {
  }

  Pos pos() const {
    return Pos(x, y);
  }

  bool operator==(Decision d) const {
    return x == d.x && y == d.y && r == d.r;
  }
  bool operator!=(Decision d) const {
    return !operator==(d);
  }

  int x, y, r;
};

namespace std {
template <> struct hash<Decision> {
  size_t operator()(const Decision& d) const {
    return d.r * 17161 + d.y * 131 + d.x;
  }
};
}

class Unit {
 public:
  Unit(const vector<Pos>& members, Pos pivot, int width)
      : members_(members), pivot_(pivot) {
    int min_x = width;
    int max_x = 0;
    for (const Pos& p : members) {
      min_x = min(min_x, p.x);
      max_x = max(max_x, p.x);
    }
    base_x_ = (width - (max_x - min_x + 1)) / 2;
  }

  const vector<Pos>& members() const { return members_; }
  Pos pivot() const { return pivot_; }
  Decision origin() const { return Decision(base_x_, 0, 0); }

 private:
  vector<Pos> members_;
  Pos pivot_;
  int base_x_;
};

class Board {
 public:
  Board(int width, int height, const vector<Pos>& filled) {
    W = width;
    H = height;
    b_.resize(W * H);
    for (Pos p : filled) {
      Fill(p);
    }
  }

  void Show() const {
    for (int y = 0; y < H; y++) {
      if (y % 2)
        fprintf(stderr, " ");
      for (int x = 0; x < W; x++) {
        fprintf(stderr, "|");
        if (At(Pos(x, y))) {
          fprintf(stderr, "X");
        } else {
          fprintf(stderr, " ");
        }
      }
      fprintf(stderr, "|\n");
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

  bool At(Pos p) const {
    if (p.x < 0 || p.x >= W || p.y < 0 || p.y >= H)
      return true;
    size_t i = p.y * W + p.x;
    assert(i < b_.size());
    return b_[i];
  }

  bool CanFill(Pos p) const {
    return !At(p);
  }

  bool CanPut(const Unit& u, const Decision& d) {
    for (Pos p : u.members()) {
      Pos np = p.Rotate(d.y, d.r, u.pivot());
      np += Pos(d.x, d.y);
      if (!CanFill(np))
        return false;
    }
    return true;
  }

  void Put(const Unit& u, const Decision& d) {
    for (Pos p : u.members()) {
      Pos np = p.Rotate(d.y, d.r, u.pivot());
      np += Pos(d.x, d.y);
      Fill(np);
    }
  }

  void GetPossibleDecisionsWithComandsImpl(
      const Unit& u,
      Decision d,
      Decision pd,
      vector<Command>* commands,
      unordered_set<Decision>* seen,
      unordered_map<Decision, vector<Command>>* out) {
    //fprintf(stderr, "%d %d %d %zu\n", d.x, d.y, d.r, seen->size());
    if (!seen->insert(d).second)
      return;

    if (!CanPut(u, d)) {
      assert(d != pd);
      out->emplace(pd, *commands);
      return;
    }

#define NEXT(nd, cmd) do {                                              \
      commands->push_back(cmd);                                         \
      GetPossibleDecisionsWithComandsImpl(u, nd, d, commands, seen, out); \
      commands->pop_back();                                             \
    } while (0)
    NEXT(Decision(d.x - 1, d.y, d.r), MOVE_W);
    NEXT(Decision(d.x + 1, d.y, d.r), MOVE_E);
    NEXT(Decision(d.pos().MoveSW(), d.r), MOVE_SW);
    NEXT(Decision(d.pos().MoveSE(), d.r), MOVE_SE);
    NEXT(Decision(d.x, d.y, (d.r + 1) % 6), ROT_C);
    NEXT(Decision(d.x, d.y, (d.r + 5) % 6), ROT_CC);
#undef NEXT
  }

  void GetPossibleDecisionsWithComands(
      const Unit& u,
      unordered_map<Decision, vector<Command>>* out) {
     Decision d = u.origin();
     vector<Command> commands;
     unordered_set<Decision> seen;
     GetPossibleDecisionsWithComandsImpl(u, d, d, &commands, &seen, out);
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
      vector<Pos> members;
      for (const auto& m : u.first) {
        members.push_back(Pos(m));
      }
      units_.push_back(Unit(members, Pos(u.second), W));
    }
    for (const auto& p : problem.filled) {
      filled_.push_back(Pos(p));
    }
  }

  void Play() {
    board_.reset(new Board(W, H, filled_));
    turn_ = 0;
    score_ = 0;

    while (true) {
      turn_++;

      const Unit& u = units_[lcg_.GetNext() % units_.size()];
      if (!board_->CanPut(u, u.origin())) {
        break;
      }

      unordered_map<Decision, vector<Command>> decisions;
      board_->GetPossibleDecisionsWithComands(u, &decisions);
      assert(!decisions.empty());

      // TODO: Eval
      Decision decision = decisions.begin()->first;
      board_->Put(u, decision);
      const vector<Command>& cmds = decisions.begin()->second;
      copy(cmds.begin(), cmds.end(), back_inserter(commands_));
      score_ += u.members().size();
      // TODO: Line removal.

      fprintf(stderr, "Turn %d s=%d d=%d,%d,%d c=%s\n",
              turn_, score_, decision.x, decision.y, decision.r,
              MakeCommandStr(cmds).c_str());
      board_->Show();
    }
  }

  const vector<Command>& commands() const { return commands_; }

 private:
  int H, W;
  int id_;
  int source_length_;
  Lcg lcg_;
  vector<Pos> filled_;
  vector<Unit> units_;

  unique_ptr<Board> board_;
  int turn_;
  vector<Command> commands_;
  int score_;
};

int main(int argc, char* argv[]) {
  Pos::Test();
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
  unordered_map<int, string> solutions;
  for (int seed : problem.source_seeds) {
    Game game(problem, seed);
    game.Play();
    bool ok = solutions.emplace(seed, MakeCommandStr(game.commands())).second;
    assert(ok);
  }

  printf("[");
  bool is_first = true;
  for (const auto& p : solutions) {
    if (!is_first)
      printf(",");
    is_first = false;
    printf("{\"problemId\":%d,\"seed\":%d,\"solution\":\"%s\"}",
           problem.id, p.first, p.second.c_str());
  }
  printf("]");
}
