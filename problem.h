#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <assert.h>
#include <math.h>

#include <vector>
#include <utility>

using namespace std;

struct Problem {
  typedef pair<int, int> Pos;
  typedef pair<vector<Pos>, Pos> Unit;

  int height, width, id, source_length;
  vector<int> source_seeds;
  vector<Pos> filled;
  vector<Unit> units;

  explicit Problem(const char* filename);
};

#endif
