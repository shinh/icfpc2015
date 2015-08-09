#ifndef SOLVER_H_
#define SOLVER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "hc.h"

using namespace std;

class Lcg;
class Problem;

class SolverBase {
 public:
  void Init(const Problem& problem, int seed, int game_index,
            const vector<string>& phrases);

  virtual string Play() = 0;

 protected:
  int GetPhraseScore(const string& commands) const;

  bool MakeNiceCommandStrFrom(const Unit& u, Decision d,
                              Decision goal,
                              map<DecisionId, Decision>* seen,
                              string* out);
  bool MakeNiceCommandAfterPhrase(const string& phrase,
                                  const Unit& u,
                                  Decision goal,
                                  string* ret);
  string MakeNiceCommandStr(const Unit& u, Decision goal);

  int H, W;
  int id_;
  int game_index_;
  int source_length_;
  unique_ptr<Lcg> lcg_;
  vector<Pos> filled_;
  vector<Unit> units_;
  vector<string> phrases_;

  unique_ptr<Board> board_;
  set<string> use_once_phrases_;
};

SolverBase* MakeNaiveSolver();

#endif
