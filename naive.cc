#include "game.h"
#include "solver.h"

class NaiveSolver : public SolverBase {
 public:
  virtual string Play() {
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

      int uid = lcg_->GetNext() % units_.size();
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

    int phrase_score = GetPhraseScore(commands_);

    fprintf(stderr, "turn=%d/%d score=%d (%d+%d)\n",
            turn_, source_length_,
            score_ + phrase_score, score_, phrase_score);
    return commands_;
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

 private:
  int turn_;
  string commands_;
  int score_;
};

SolverBase* MakeNaiveSolver() {
  return new NaiveSolver();
}
