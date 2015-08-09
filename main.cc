#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
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

Command GetCommandFromChar(char c) {
  switch (c) {
    case 'p':
    case '\'':
    case '!':
    case '.':
    case '0':
    case '3':
      return MOVE_W;
    case 'b':
    case 'c':
    case 'e':
    case 'f':
    case 'y':
    case '2':
      return MOVE_E;
    case 'a':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case '4':
      return MOVE_SW;
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case ' ':
    case '5':
      return MOVE_SE;
    case 'd':
    case 'q':
    case 'r':
    case 'v':
    case 'z':
    case '1':
      return ROT_C;
    case 'k':
    case 's':
    case 't':
    case 'u':
    case 'w':
    case 'x':
      return ROT_CC;
    default:
      assert(false);
  }
}

string MakeCommandStr(const vector<Command>& cmds) {
  string r;
  for (Command c : cmds) {
    switch (c) {
      case MOVE_W:
        r += '!';
        break;
      case MOVE_E:
        r += 'e';
        break;
      case MOVE_SW:
        r += 'i';
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

  bool operator<(const Pos& p) const {
    if (x < p.x)
      return true;
    if (x > p.x)
      return false;
    return y < p.y;
  }

  bool operator>(const Pos& p) const {
    if (x > p.x)
      return true;
    if (x < p.x)
      return false;
    return y > p.y;
  }

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

  Pos StepSW(int n) const {
    return Pos(x - n / 2 + (y % 2 - 1) * (n % 2), y + n);
  }

  Pos StepSE(int n) const {
    return Pos(x + n / 2 + (y % 2) * (n % 2), y + n);
  }

  Pos StepNW(int n) const {
    return Pos(x - n / 2 + (y % 2 - 1) * (n % 2), y - n);
  }

  Pos StepNE(int n) const {
    return Pos(x + n / 2 + (y % 2) * (n % 2), y - n);
  }

  Pos Rotate(/*int cy, */int r, Pos p) {
    // E, SE, SW, W, NW, NE
    int moves[6] = {};
    Pos m = Pos(p.x, p.y);
    if (y >= p.y) {
      int se_cnt = y - p.y;
      m = m.StepSE(se_cnt);
      moves[1] = se_cnt;
    } else {
      int ne_cnt = p.y - y;
      m = m.StepNE(ne_cnt);
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
    m = m.StepSE(rmoves[1]);
    m = m.StepSW(rmoves[2]);
    m = m.StepNW(rmoves[4]);
    m = m.StepNE(rmoves[5]);

    Pos ret = Pos(m.x, m.y);
#if 0
    //if (r == 0 && *this != ret || true) {
    if (r == 0 && *this != ret) {
      fprintf(stderr, "Rotate (%d,%d) pivot=(%d,%d) rot=%d => (%d,%d)\n",
              x, y, p.x, p.y, r, ret.x, ret.y);
      fprintf(stderr, "moves={%d,%d,%d,%d,%d,%d} rmoves={%d,%d,%d,%d,%d,%d}\n",
              moves[0], moves[1], moves[2], moves[3], moves[4], moves[5],
              rmoves[0], rmoves[1], rmoves[2],
              rmoves[3], rmoves[4], rmoves[5]);
    }
#endif

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

  Decision Move(Command cmd) {
    switch (cmd) {
      case MOVE_W:
        return Decision(x - 1, y, r);
      case MOVE_E:
        return Decision(x + 1, y, r);
      case MOVE_SW:
        return Decision(pos().MoveSW(), r);
      case MOVE_SE:
        return Decision(pos().MoveSE(), r);
      case ROT_C:
        return Decision(x, y, (r + 1) % 6);
      case ROT_CC:
        return Decision(x, y, (r + 5) % 6);
      default:
        assert(false);
    }
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
    base_x_ = (width - (max_x - min_x + 1)) / 2 - min_x; // min_x is not always 0, therefore have to minus min_x.
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

struct DecisionId {
  DecisionId(const Unit& u, Decision d) {
    p = d.Apply(u.pivot(), u.pivot());
    for (Pos p : u.members()) {
      Pos np = d.Apply(p, u.pivot());
      m.insert(np);
    }
  }

  bool operator<(const DecisionId d) const {
    if (p < d.p)
      return true;
    if (p > d.p)
      return false;
    return m < d.m;
  }

  bool operator==(const DecisionId d) const {
    return p == d.p && m == d.m;
  }

  Pos p;
  set<Pos> m;
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

#if 0
  int GetPosId(Pos p) {
    // +2 for boundaries.
    return p.y * (1000) + p.x;
  }
#else
  pair<int, int> GetPosId(Pos p) {
    return make_pair(p.x, p.y);
  }
#endif

  void GetDecisionId(const Unit& u, Decision d, set<Pos>* id) {
    id->insert(u.pivot());
    for (Pos p : u.members()) {
      Pos np = d.Apply(p, u.pivot());
      id->insert(np);
    }
  }

  void GetPossibleDecisionsImpl(const Unit& u,
                                Decision d,
                                Decision pd,
                                set<Decision>* seen,
                                set<Decision>* out) {
    //fprintf(stderr, "%d %d %d %zu\n", d.x, d.y, d.r, seen->size());
    if (!CanPut(u, d)) {
      assert(d != pd);
      out->emplace(pd);
      return;
    }

    if (!seen->insert(d).second)
      return;

#define NEXT(nd) do {                                              \
      GetPossibleDecisionsImpl(u, nd, d, seen, out);               \
    } while (0)
    NEXT(Decision(d.x - 1, d.y, d.r));
    NEXT(Decision(d.x + 1, d.y, d.r));
    NEXT(Decision(d.pos().MoveSW(), d.r));
    NEXT(Decision(d.pos().MoveSE(), d.r));
    NEXT(Decision(d.x, d.y, (d.r + 1) % 6));
    NEXT(Decision(d.x, d.y, (d.r + 5) % 6));
#undef NEXT
  }

  void GetPossibleDecisions(const Unit& u, set<Decision>* out) {
     Decision d = u.origin();
     set<Decision> seen;
     GetPossibleDecisionsImpl(u, d, d, &seen, out);
  }

 private:
  int W, H;
  vector<int> b_;
};

class Game {
 public:
  Game(const Problem& problem, int seed, int game_index,
       const vector<string>& phrases)
      : lcg_(seed),
        phrases_(phrases) {
    H = problem.height;
    W = problem.width;
    id_ = problem.id;
    game_index_ = game_index;
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

    use_once_phrases_.insert("yuggoth");
    use_once_phrases_.insert("ia! ia!");
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

      set<Decision> decisions;
      board_->GetPossibleDecisions(u, &decisions);
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

      Decision decision = ChooseBest(u, decisions);

      string cmds = MakeNiceCommandStr(u, decision);
      commands_ += cmds;

      board_->Put(u, decision);
      board_->Show(u, decision);

      int ls = board_->Clear();
      // TODO: old_ls
      //const vector<Command>& cmds = d.second;
      //copy(cmds.begin(), cmds.end(), back_inserter(commands_));
      if (ls > 0) {
        fprintf(stderr, "line remove! %d\n", ls);
      }
      score_ += u.members().size() + 100 * (1 + ls) * ls / 2;

      fprintf(stderr, "Game %d Turn %d s=%d u=%d d=%d,%d,%d c=%s n=%zu\n",
              game_index_, turn_, score_, uid,
              decision.x, decision.y, decision.r,
              cmds.c_str(),
              decisions.size());
    }

    int phrase_score = GetPhraseScore();

    fprintf(stderr, "turn=%d/%d score=%d (%d+%d)\n",
            turn_, source_length_,
            score_ + phrase_score, score_, phrase_score);
  }

  int GetPhraseScore() const {
    int score = 0;
    string cmd = commands_;
    for (string phrase : phrases_) {
      bool is_first = true;
      size_t index = 0;
      while (index < cmd.size()) {
        index = cmd.find(phrase, index);
        if (index == string::npos)
          break;
        index++;

        if (is_first) {
          score += 300;
          is_first = false;
        }
        score += 2 * phrase.size();
      }
    }
    return score;
  }

  Decision ChooseBest(const Unit& u, const set<Decision>& decisions) {
    double best_score = -1e99;
    Decision best_decision;

    for (Decision d : decisions) {
      Board nb = *board_;
      nb.Put(u, d);
      double score = nb.Eval();
      if (best_score < score) {
        best_score = score;
        best_decision = d;
      }
    }

    return best_decision;
  }

  const string& commands() const { return commands_; }

  struct MakeNiceCommandCtx {
    Decision d;
    Decision pd;
    vector<Command> cmds;
    int bap_bonus;
  };

  bool MakeNiceCommandStrFrom(const Unit& u, Decision d, Decision goal,
                              map<DecisionId, Decision>* seen,
                              string* out) {
    multimap<int, MakeNiceCommandCtx> q;
    MakeNiceCommandCtx ctx;
    ctx.d = d;
    ctx.pd = d;
    ctx.bap_bonus = 0;
    q.emplace(0, ctx);

    while (!q.empty()) {
      int prio = q.begin()->first;
      const auto& ctx = q.begin()->second;
      Decision d = ctx.d;
      Decision pd = ctx.pd;
      //fprintf(stderr, "%d,%d,%d => %d,%d,%d\n", pd.x, pd.y, pd.r, d.x, d.y, d.r);
      vector<Command> cmds = ctx.cmds;
      q.erase(q.begin());

      if (!board_->CanPut(u, d)) {
        if (DecisionId(u, pd) == DecisionId(u, goal)) {
          *out += MakeCommandStr(cmds);
          return true;
        }
        //fprintf(stderr, "cannot put...\n");
        continue;
      }

#if 0
      set<Pos> decision_id;
      board_->GetDecisionId(u, d, &decision_id);
      if (!seen->insert(decision_id).second)
        continue;

      if (!seen->insert(d).second)
        continue;
#endif

      if (!seen->emplace(DecisionId(u, d), d).second) {
        continue;
      }

      bool did_bap = false;
      {
        Decision bd = Decision(d.x + 1, d.y, d.r);
        Decision ad = Decision(bd.pos().MoveSW(), bd.r);
        Decision pd = Decision(ad.x - 1, ad.y, ad.r);
        if (prio > -100000 &&
            board_->CanPut(u, bd) &&
            board_->CanPut(u, ad) &&
            board_->CanPut(u, pd) &&
            !seen->count(DecisionId(u, bd)) &&
            !seen->count(DecisionId(u, ad)) &&
            !seen->count(DecisionId(u, pd))) {
          did_bap = true;
          MakeNiceCommandCtx nctx;

          nctx.d = pd;
          nctx.pd = ad;
          nctx.bap_bonus = ctx.bap_bonus - 100;
          nctx.cmds = cmds;
          nctx.cmds.push_back(MOVE_E);
          nctx.cmds.push_back(MOVE_SW);
          nctx.cmds.push_back(MOVE_W);
          int dist = abs(pd.x - goal.x) + abs(pd.y - goal.y) + abs(pd.r - goal.r);
          q.emplace(dist + nctx.bap_bonus - 200000, nctx);

          nctx.d = ad;
          nctx.pd = bd;
          nctx.bap_bonus = ctx.bap_bonus;
          nctx.cmds = cmds;
          nctx.cmds.push_back(MOVE_E);
          nctx.cmds.push_back(MOVE_SW);
          dist = abs(ad.x - goal.x) + abs(ad.y - goal.y) + abs(ad.r - goal.r);
          q.emplace(dist + nctx.bap_bonus - 300000, nctx);
        }
      }

#define NEXT(nd, cmd, prio) do {                                        \
        MakeNiceCommandCtx nctx;                                        \
        nctx.d = nd;                                                    \
        nctx.pd = d;                                                    \
        nctx.bap_bonus = ctx.bap_bonus;                                 \
        nctx.cmds = cmds;                                               \
        nctx.cmds.push_back(cmd);                                       \
        int dist = abs(nd.x - goal.x) + abs(nd.y - goal.y) + abs(nd.r - goal.r); \
        q.emplace(dist * 2 + nctx.bap_bonus + prio, nctx);              \
      } while (0)
      NEXT(Decision(d.x - 1, d.y, d.r), MOVE_W, 10);
      NEXT(Decision(d.x + 1, d.y, d.r), MOVE_E, did_bap ? -500000 : -4);
      NEXT(Decision(d.pos().MoveSW(), d.r), MOVE_SW, 0);
      NEXT(Decision(d.pos().MoveSE(), d.r), MOVE_SE, 0);
      NEXT(Decision(d.x, d.y, (d.r + 1) % 6), ROT_C, -2);
      NEXT(Decision(d.x, d.y, (d.r + 5) % 6), ROT_CC, -2);
#undef NEXT
    }

    return false;
  }

  bool MakeNiceCommandAfterPhrase(const string& phrase,
                                  const Unit& u,
                                  Decision goal,
                                  string* ret) {
    for (int n = H - 1; n > 0; n--) {
      map<DecisionId, Decision> seen;
      string out;
      bool ok = true;
      Decision d = u.origin();
      seen.emplace(DecisionId(u, d), d);

      for (int i = 0; i < n; i++) {
        out += phrase;
        for (size_t j = 0; j < phrase.size(); j++) {
          d = d.Move(GetCommandFromChar(phrase[j]));

          if (!board_->CanPut(u, d)) {
            ok = false;
            break;
          }
          if (i < n - 1 || j < phrase.size() - 1) {
            if (!seen.emplace(DecisionId(u, d), d).second) {
              //fprintf(stderr, "%d,%d,%d\n", d.x, d.y, d.r);
              //assert(false);
              ok = false;
              break;
            }
          }
        }
        if (!ok)
          break;
      }

      if (ok) {
        //fprintf(stderr, "go %d d=%d,%d,%d\n", n, d.x, d.y, d.r);
        if (MakeNiceCommandStrFrom(u, d, goal, &seen, &out)) {
          *ret = out;
          return true;
        }
      }
    }
    return false;
  }

  string MakeNiceCommandStr(const Unit& u, Decision goal) {
#if 1
    string out;

    for (set<string>::iterator iter = use_once_phrases_.begin();
         iter != use_once_phrases_.end();
         ++iter) {
      if (MakeNiceCommandAfterPhrase(*iter, u, goal, &out)) {
        use_once_phrases_.erase(iter);
        return out;
      }
    }

    if (MakeNiceCommandAfterPhrase("r'lyeh", u, goal, &out)) {
      return out;
    }

    for (int n = H - 1; n > 0; n--) {
      map<DecisionId, Decision> seen;
      string out;
      bool ok = true;
      Decision d = u.origin();
      seen.emplace(DecisionId(u, d), d);

      for (int i = 0; i < n; i++) {
        out += "ei!";
        for (int j = 0; j < (i < n - 1 ? 4 : 3); j++) {
          if (j == 0) {
            d = Decision(d.x + 1, d.y, d.r);
          } else if (j == 1) {
            d = Decision(d.pos().MoveSW(), d.r);
          } else if (j == 2) {
            d = Decision(d.x - 1, d.y, d.r);
          } else if (j == 3) {
            Decision td;
            td = Decision(d.x, d.y, (d.r + 1) % 6);
            if (board_->CanPut(u, td) && !seen.count(DecisionId(u, td))) {
              d = td;
              out += 'd';
            } else {
              td = Decision(d.x, d.y, (d.r + 5) % 6);
              if (board_->CanPut(u, td) && !seen.count(DecisionId(u, td))) {
                d = td;
                out += 'k';
              } else {
                d = Decision(d.pos().MoveSE(), d.r);
                out += 'l';
              }
            }
          }

          if (!board_->CanPut(u, d)) {
            ok = false;
            break;
          }
          if (i < n - 1 || j < 2) {
            if (!seen.emplace(DecisionId(u, d), d).second) {
              fprintf(stderr, "%d,%d,%d\n", d.x, d.y, d.r);
              assert(false);
              ok = false;
              break;
            }
          }
        }
        if (!ok)
          break;
      }

      if (ok) {
        //fprintf(stderr, "go %d d=%d,%d,%d\n", n, d.x, d.y, d.r);
        if (MakeNiceCommandStrFrom(u, d, goal, &seen, &out))
          return out;
      }
    }
#endif

    map<DecisionId, Decision> seen;
    Decision d = u.origin();
    bool ok = MakeNiceCommandStrFrom(u, d, goal, &seen, &out);
    assert(ok);
    return out;
  }

 private:
  int H, W;
  int id_;
  int game_index_;
  int source_length_;
  Lcg lcg_;
  vector<Pos> filled_;
  vector<Unit> units_;
  vector<string> phrases_;

  unique_ptr<Board> board_;
  int turn_;
  string commands_;
  int score_;
  set<string> use_once_phrases_;
};

int main(int argc, char* argv[]) {
  Lcg::Test();
  Pos::Test();
  Decision::Test();

  vector<string> phrases;

  const char* filename = "problems/problem_0.json";
  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if (!strcmp(arg, "-f")) {
      filename = argv[++i];
    } else if (!strcmp(arg, "-p")) {
      phrases.push_back(argv[++i]);
    }
    // TODO: Implement the rest.
  }

  if (phrases.empty()) {
    // Known phrases.
    phrases.push_back("ia! ia!");
    phrases.push_back("ei!");
    phrases.push_back("r'lyeh");
    phrases.push_back("yuggoth");
  }

  Problem problem(filename);
  //unordered_map<int, string> solutions;
  vector<pair<int, string>> solutions;
  int game_index = 0;
  for (int seed : problem.source_seeds) {
    Game game(problem, seed, game_index++, phrases);
    game.Play();
    //bool ok = solutions.emplace(seed, MakeCommandStr(game.commands())).second;
    //assert(ok);
    solutions.push_back(make_pair(seed, game.commands()));
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
