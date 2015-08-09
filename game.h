#ifndef GAME_H_
#define GAME_H_

#include <assert.h>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "hc.h"

using namespace std;

class Lcg {
 public:
  explicit Lcg(int seed) {
    v_ = seed;
  }

  explicit Lcg(const Lcg& l) {
    v_ = l.v_;
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

inline Command GetCommandFromChar(char c) {
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

inline string MakeCommandStr(const vector<Command>& cmds) {
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

namespace std {
template <> struct hash<Decision> {
  size_t operator()(const Decision& d) const {
    return d.r * 17161 + d.y * 131 + d.x;
  }
};
}

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
  Board() {}

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

  bool CanPut(const Unit& u, const Decision& d) const {
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

  void GetPossibleDecisionsImpl(const Unit& u,
                                Decision d,
                                Decision pd,
                                set<Decision>* seen,
                                set<Decision>* out) const {
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

  void GetPossibleDecisions(const Unit& u, set<Decision>* out) const {
     Decision d = u.origin();
     set<Decision> seen;
     GetPossibleDecisionsImpl(u, d, d, &seen, out);
  }

 private:
  int W, H;
  vector<int> b_;
};

#endif
