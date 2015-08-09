#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "game.h"
#include "problem.h"
#include "solver.h"

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
    unique_ptr<SolverBase> solver(MakeNaiveSolver());
    solver->Init(problem, seed, game_index++, phrases);
    const string& commands = solver->Play();
    solutions.push_back(make_pair(seed, commands));
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
