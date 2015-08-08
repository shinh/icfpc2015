#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "problem.h"

typedef unordered_set<int> IntSet;
namespace std {
template <> struct hash<IntSet> {
  size_t operator()(const IntSet& s) const {
    size_t r = 0;
    for (int v : s) {
      r = (r * 131) + v;
    }
    return r;
  }
};
}

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
#if 0
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

    CHECK_EQ(Pos(0, 1).Rotate(0, 0, Pos(0, 0)), Pos(0, 1));

    CHECK_EQ(Pos(0, 1).Rotate(0, 0, Pos(1, 1)), Pos(0, 1));

    CHECK_EQ(Pos(0, 0).Rotate(3, 1, Pos(0, 0)), Pos(0, 0));
    CHECK_EQ(Pos(0, 1).Rotate(3, 1, Pos(0, 0)), Pos(0, 1));
    CHECK_EQ(Pos(0, 2).Rotate(3, 1, Pos(0, 0)), Pos(-1, 1));
#undef CHECK_EQ
#endif
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

  Pos MoveNW() const {
    return Pos(x - 1 + y % 2, y - 1);
  }

  Pos MoveNE() const {
    return Pos(x + y % 2, y - 1);
  }

  Pos Rotate(/*int cy, */int r, Pos p) {
    // E, SE, SW, W, NW, NE
    int moves[6] = {};
    Pos m = Pos(p.x, p.y);
    if (y >= p.y) {
      int se_cnt = 0;
      while (y != m.y) {
        m = m.MoveSE();
        se_cnt++;
      }
      moves[1] = se_cnt;
    } else {
      int ne_cnt = 0;
      while (y != m.y) {
        m = m.MoveNE();
        ne_cnt++;
      }
      moves[5] = ne_cnt;
    }

    if (x >= m.x) {
      moves[0] = x - m.x;
    } else {
      moves[3] = m.x - x;
    }

    int rmoves[6];
    for (int i = 0; i < 6; i++) {
      int v = moves[i];
      rmoves[(i + r) % 6] = v;
    }

    m = Pos(p.x, p.y);
    m.x += rmoves[0];
    m.x -= rmoves[3];
    for (int i = 0; i < rmoves[1]; i++)
      m = m.MoveSE();
    for (int i = 0; i < rmoves[2]; i++)
      m = m.MoveSW();
    for (int i = 0; i < rmoves[4]; i++)
      m = m.MoveNW();
    for (int i = 0; i < rmoves[5]; i++)
      m = m.MoveNE();

    Pos ret = Pos(m.x, m.y);
    //if (r == 0 && *this != ret || true) {
    if (r == 0 && *this != ret) {
      fprintf(stderr, "Rotate (%d,%d) pivot=(%d,%d) rot=%d => (%d,%d)\n",
              x, y, p.x, p.y, r, ret.x, ret.y);
      fprintf(stderr, "moves={%d,%d,%d,%d,%d,%d} rmoves={%d,%d,%d,%d,%d,%d}\n",
              moves[0], moves[1], moves[2], moves[3], moves[4], moves[5],
              rmoves[0], rmoves[1], rmoves[2],
              rmoves[3], rmoves[4], rmoves[5]);
    }

    return ret;

#if 0
    fprintf(stderr, "Rotate (%d,%d) pivot=(%d,%d) rot=%d y=%d\n",
            x, y, p.x, p.y, r, cy);
    double dy = sqrt(1-0.5*0.5);
    double pgx = p.GetGeomX(cy);
    double gx = GetGeomX(cy) - pgx;
    double gy = (y - p.y) * dy;
    double gr = M_PI * 2 * r / 6;
    double nx = gx * cos(gr) - gy * sin(gr);
    double ny = gx * sin(gr) + gy * cos(gr);
    int iy = round(ny / dy) + p.y;
    int ix = round(nx + pgx - ((iy + cy) & 1) * 0.5);
    //int ix = ceil(nx + pgx);
    //int ix = round(nx + pgx - ((iy + cy) % 2) * 0.5);
    //int ix = round(nx + pgx - (iy % 2) * 0.5);
    fprintf(stderr, "(%f,%f) => (%f,%f) %d,%d\n", gx, gy, nx, ny, ix, iy);
    return Pos(ix, iy);
#endif

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
  Decision()
      : x(-1), y(-1), r(-1) {
  }

  Pos pos() const {
    return Pos(x, y);
  }

  static void Test() {
#define CHECK_EQ(a, b) Pos::CheckEq(a, b, __FILE__, __LINE__)

#if 0
    for (int y = 0; y < 10; y++) {
      for (int x = 0; x < 10; x++) {
        CHECK_EQ(Decision(x, y, 0).Apply(Pos(0, 0), Pos(0, 0)), Pos(x, y));
      }
    }
#endif

    CHECK_EQ(Decision(-1, 1, 0).Apply(Pos(0, 0), Pos(0, 0)), Pos(-1, 1));

#undef CHECK_EQ
  }

  Pos Apply(Pos p, Pos pivot) const {
#if 0
#define VERBOSE_APPLY

#ifdef VERBOSE_APPLY
    fprintf(stderr, "Apply (%d,%d) pivot=(%d,%d) x=%d y=%d rot=%d\n",
            p.x, p.y, pivot.x, pivot.y, x, y, r);
#endif

    static const double kDeltaY = sqrt(1-0.5*0.5);
    double gy = p.y * kDeltaY;
    double gx = p.GetGeomX(y);
    double pgy = pivot.y * kDeltaY;
    double pgx = pivot.GetGeomX(y);

    double dy = gy - pgy;
    double dx = gx - pgx;
    double gr = M_PI * 2 * r / 6;
    double ndx = dx * cos(gr) - dy * sin(gr);
    double ndy = dx * sin(gr) + dy * cos(gr);
#ifdef VERBOSE_APPLY
    fprintf(stderr, "d=(%f,%f) => nd=(%f,%f)\n", dx, dy, ndx, ndy);
#endif

    double ngx = pgx + ndx;
    double ngy = pgy + ndy;

    int dxa = int(round(ngy / kDeltaY)) % 2;

    ngy += y * kDeltaY;

    int dxb = int(round(ngy / kDeltaY)) % 2;

    int iy = round(ngy / kDeltaY);

    fprintf(stderr, "dxa=%d dxb=%d\n", dxa, dxb);
    //int ix = round(ngx + x + (dxb - dxa) * 0.5);
    int ix = round(ngx + x + (dxa - dxb) * 0.5);

    //ngx += x;
    //int ix = round(ngx - iy % 2 * 0.5);
    //int ix = floor(ngx + 0.001);

#ifdef VERBOSE_APPLY
    fprintf(stderr, "(%f,%f) => (%f,%f)\n", gx, gy, ngx, ngy);
#endif
    return Pos(ix, iy);

#else

    int nx = p.x + x;
    int ny = p.y + y;
    if (p.y % 2)
      nx += 1 - (ny & 1);

    int npx = pivot.x + x;
    int npy = pivot.y + y;
    if (pivot.y % 2)
      npx += 1 - (npy & 1);

    return Pos(nx, ny).Rotate(r, Pos(npx, npy));

#if 0
    Pos np = p.Rotate(y, r, pivot);
    int nx = x + np.x;
    int ny = y + np.y;
    int dx = 0;
    if (np.y % 2)
      dx = 1 - ny % 2;
    //int dx = np.y % 2 - ny % 2;
    //fprintf(stderr, "np=(%d,%d) %d %d %d\n", np.x, np.y, nx, ny, dx);
    return Pos(nx + dx, ny);
#endif

#endif
  }

  bool operator==(Decision d) const {
    return x == d.x && y == d.y && r == d.r;
  }
  bool operator!=(Decision d) const {
    return !operator==(d);
  }

  bool operator<(Decision d) const {
    if (x < d.x)
      return true;
    if (x > d.x)
      return false;
    if (y < d.y)
      return true;
    if (y > d.y)
      return false;
    return r < d.r;
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
  int base_x() const { return base_x_; }

 private:
  vector<Pos> members_;
  Pos pivot_;
  int base_x_;
};

//typedef unordered_map<Decision, vector<Command>> DecisionMap;
typedef map<Decision, vector<Command>> DecisionMap;

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

  void Show(const Unit& u, Decision d) const {
    vector<Pos> poses;
    for (Pos p : u.members()) {
      Pos np = d.Apply(p, u.pivot());
      poses.push_back(np);
    }

    for (int y = 0; y < H; y++) {
      if (y % 2)
        fprintf(stderr, " ");
      for (int x = 0; x < W; x++) {
        fprintf(stderr, "|");
        Pos p = Pos(x, y);
        if (find(poses.begin(), poses.end(), p) != poses.end()) {
          fprintf(stderr, "o");
        } else if (At(p)) {
          fprintf(stderr, "X");
        } else {
          fprintf(stderr, " ");
        }
      }
      fprintf(stderr, "|\n");
    }
  }

  void Set(Pos p, bool b) {
    assert(p.x >= 0);
    assert(p.x < W);
    assert(p.y >= 0);
    assert(p.y < H);
    size_t i = p.y * W + p.x;
    assert(i < b_.size());
    b_[i] = b;
  }

  void Fill(Pos p) {
    Set(p, true);
  }

  bool At(int x, int y) const {
    size_t i = y * W + x;
    assert(i < b_.size());
    return b_[i];
  }

  bool At(Pos p) const {
    return At(p.x, p.y);
  }

  bool CanFill(Pos p) const {
    if (p.x < 0 || p.x >= W || p.y < 0 || p.y >= H)
      return false;
    return !At(p);
  }

  bool CanPut(const Unit& u, const Decision& d) {
    for (Pos p : u.members()) {
      Pos np = d.Apply(p, u.pivot());
      if (!CanFill(np))
        return false;
    }
    return true;
  }

  void Put(const Unit& u, const Decision& d) {
    for (Pos p : u.members()) {
      Pos np = d.Apply(p, u.pivot());
      Fill(np);
    }
  }

  int Clear() {
    int n = 0;
    for (int y = 0; y < H; y++) {
      bool ok = true;
      for (int x = 0; x < W; x++) {
        if (!At(Pos(x, y))) {
          ok = false;
          break;
        }
      }
      if (ok) {
        n++;
        for (int dy = y; dy > 0; dy--) {
          for (int x = 0; x < W; x++) {
            Set(Pos(x, dy), At(x, dy - 1));
          }
        }
        for (int x = 0; x < W; x++) {
          Set(Pos(x, 0), false);
        }
      }
    }
    return n;
  }

  double Eval() {
    int ls = Clear();
    double score = 100 * (1 + ls) * ls / 2;

    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        if (!At(x, y)) {
          continue;
        }

        int dy = H - y;
        int dx = min(x, W - x - 1);
        score -= 0.1 * dy;
        score -= 0.001 * dx;
      }
    }

    return score;
  }

  int GetPosId(Pos p) {
    // +2 for boundaries.
    return p.y * (W + 2) + p.x;
  }

  void GetDecisionId(const Unit& u, Decision d, IntSet* id) {
    id->insert(GetPosId(u.pivot()));
    for (Pos p : u.members()) {
      Pos np = d.Apply(p, u.pivot());
      id->insert(GetPosId(np));
    }
  }

  void GetPossibleDecisionsWithComandsImpl(const Unit& u,
                                           Decision d,
                                           Decision pd,
                                           vector<Command>* commands,
                                           unordered_set<IntSet>* seen,
                                           DecisionMap* out) {
    //fprintf(stderr, "%d %d %d %zu\n", d.x, d.y, d.r, seen->size());
    if (!CanPut(u, d)) {
      assert(d != pd);
      out->emplace(pd, *commands);
      return;
    }

    IntSet decision_id;
    GetDecisionId(u, d, &decision_id);
    if (!seen->insert(decision_id).second)
      return;

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

  void GetPossibleDecisionsWithComands(const Unit& u, DecisionMap* out) {
     Decision d = u.origin();
     vector<Command> commands;
     unordered_set<IntSet> seen;
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

#if 0
    {
      const Unit& u = units_[6];
      fprintf(stderr, "===\n");
      board_->Show(u, Decision(0, 9, 0));
      fprintf(stderr, "===\n");
      board_->Show(u, Decision(0, 9, 1));
      fprintf(stderr, "===\n");
      board_->Show(u, Decision(-1, 9, 0));
      fprintf(stderr, "===\n");
      board_->Show(u, Decision(-1, 9, 1));
      fprintf(stderr, "===\n");
    }
#endif

#if 0
    {
      const Unit& u = units_[12];
      board_->Show(u, u.origin());
      for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
          for (int r = 0; r < 6; r++) {
            fprintf(stderr, "%d,%d,%d\n", x, y, r);
            board_->Show(u, Decision(x, y, r));
          }
        }
      }
#if 0
      board_->Show(u, u.origin());
      board_->Show(u, Decision(u.base_x() + 4, 0, 0));
      fprintf(stderr, "%d\n", board_->CanPut(u, Decision(4, 0, 0)));
#endif
    }
#endif

    while (true) {
      turn_++;
      if (turn_ > source_length_)
        break;

      int uid = lcg_.GetNext() % units_.size();
      const Unit& u = units_[uid];
      if (!board_->CanPut(u, u.origin())) {
        fprintf(stderr, "dead :( u=%d bx=%d\n", uid, u.base_x());
        board_->Show(u, u.origin());
        break;
      }

      DecisionMap decisions;
      board_->GetPossibleDecisionsWithComands(u, &decisions);
      assert(!decisions.empty());

#if 0
      for (const auto& p : decisions) {
        Board nb = *board_;
        Decision d = p.first;
        nb.Put(u, d);
        fprintf(stderr, "decision %d,%d,%d %s\n",
                d.x, d.y, d.r,
                MakeCommandStr(p.second).c_str());
        nb.Show();
      }
#endif

      auto d = ChooseBest(u, decisions);

      // TODO: Eval
      Decision decision = d.first;
      board_->Put(u, decision);
      board_->Show(u, decision);

      int ls = board_->Clear();
      // TODO: old_ls
      const vector<Command>& cmds = d.second;
      copy(cmds.begin(), cmds.end(), back_inserter(commands_));
      score_ += u.members().size() + 100 * (1 + ls) * ls / 2;

      fprintf(stderr, "Turn %d s=%d u=%d d=%d,%d,%d c=%s n=%zu\n",
              turn_, score_, uid, decision.x, decision.y, decision.r,
              MakeCommandStr(cmds).c_str(),
              decisions.size());
    }

    fprintf(stderr, "turn=%d/%d score=%d\n", turn_, source_length_, score_);
  }

  pair<Decision, vector<Command>> ChooseBest(const Unit& u,
                                             DecisionMap decisions) {
    double best_score = -1e99;
    Decision best_decision;

    for (const auto& p : decisions) {
      Decision d = p.first;

      Board nb = *board_;
      nb.Put(u, d);
      double score = nb.Eval();
      if (best_score < score) {
        best_score = score;
        best_decision = d;
      }
    }

    auto found = decisions.find(best_decision);
    assert(found != decisions.end());
    return *found;
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
  Lcg::Test();
  Pos::Test();
  Decision::Test();

  const char* filename = "problems/problem_0.json";
  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if (!strcmp(arg, "-f")) {
      filename = argv[++i];
    }
    // TODO: Implement the rest.
  }

  Problem problem(filename);
  //unordered_map<int, string> solutions;
  vector<pair<int, string>> solutions;
  for (int seed : problem.source_seeds) {
    Game game(problem, seed);
    game.Play();
    //bool ok = solutions.emplace(seed, MakeCommandStr(game.commands())).second;
    //assert(ok);
    solutions.push_back(make_pair(seed, MakeCommandStr(game.commands())));
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
