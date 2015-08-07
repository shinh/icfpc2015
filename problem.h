#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <vector>
#include <utility>

using namespace std;

struct Pos {
  int x, y;
  Pos() : x(-1), y(-1) {}
  Pos(int x0, int y0) : x(x0), y(y0) {}
};

struct Problem {
  int height, width, id, source_length;
  vector<int> source_seeds;
  vector<Pos> filled;
  typedef pair<vector<Pos>, Pos> Unit;
  vector<Unit> units;

  explicit Problem(const char* filename);
};

#endif
