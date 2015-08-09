#ifndef HC_H_
#define HC_H_

#include <assert.h>

#include <vector>
#include <utility>

using namespace std;

enum Command {
  MOVE_W,
  MOVE_E,
  MOVE_SW,
  MOVE_SE,
  ROT_C,
  ROT_CC,
};

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

#endif
