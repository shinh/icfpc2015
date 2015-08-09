#include "game.h"
#include "solver.h"

class SearchSolver : public SolverBase {
 public:
  explicit SearchSolver(int depth)
    : depth_(depth) {
  }

  struct Plan {
    explicit Plan(const Lcg& l)
      : lcg(l), score(0), dead(false) {
    }

    vector<Decision> decisions;
    Board board;
    Lcg lcg;
    int score;
    bool dead;
  };

  void GetAllPlansImpl(const Unit& u, const Plan& plan, int depth,
                       vector<Plan*>* out) {
    if (depth == 0) {
      out->push_back(new Plan(plan));
      return;
    }

    if (!plan.board.CanPut(u, u.origin())) {
      Plan* p = new Plan(plan);
      p->dead = true;
      out->push_back(new Plan(plan));
      return;
    }

    set<Decision> decisions;
    plan.board.GetPossibleDecisions(u, &decisions);
    for (Decision d : decisions) {
      Plan nplan = plan;
      nplan.decisions.push_back(d);
      nplan.board.Put(u, d);
      int ls = board_->Clear();
      nplan.score += 100 * (1 + ls) * ls / 2;
      const Unit& nu = units_[nplan.lcg.GetNext() % units_.size()];
      GetAllPlansImpl(nu, nplan, depth - 1, out);
    }
  }

  void GetAllPlans(const Unit& u, vector<Plan*>* out) {
    Plan plan(*lcg_);
    plan.board = *board_;
    GetAllPlansImpl(u, plan, depth_, out);
  }

  virtual string Play() {
    board_.reset(new Board(W, H, filled_));
    turn_ = 0;
    score_ = 0;

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

      vector<Plan*> plans;
      GetAllPlans(u, &plans);
      assert(!plans.empty());

      Decision decision = ChooseBest(plans);

      string cmds = MakeNiceCommandStr(u, decision);
      commands_ += cmds;

      board_->Put(u, decision);
      board_->Show(u, decision);

      int ls = board_->Clear();
      if (ls > 0) {
        fprintf(stderr, "line remove! %d\n", ls);
      }
      score_ += u.members().size() + 100 * (1 + ls) * ls / 2;

      fprintf(stderr, "Game %d Turn %d s=%d u=%d d=%d,%d,%d c=%s n=%zu\n",
              game_index_, turn_, score_, uid,
              decision.x, decision.y, decision.r,
              cmds.c_str(),
              plans.size());

      for (Plan* p : plans)
        delete p;

      report_fn_(commands_, score_, sr_);
    }

    int phrase_score = GetPhraseScore(commands_);

    fprintf(stderr, "turn=%d/%d score=%d (%d+%d)\n",
            turn_, source_length_,
            score_ + phrase_score, score_, phrase_score);

    report_fn_(commands_, score_, sr_);

    return commands_;
  }

  Decision ChooseBest(const vector<Plan*>& plans) {
    double best_score = -1e99;
    Decision best_decision;

    for (Plan* p : plans) {
      double score = p->board.Eval();
      score += p->score;
      if (p->dead)
        score -= 1000000;
      if (best_score < score) {
        best_score = score;
        best_decision = p->decisions.front();
      }
    }

    return best_decision;
  }

 private:
  int turn_;
  string commands_;
  int score_;
  int depth_;
};

SolverBase* MakeSearchSolver(int depth) {
  return new SearchSolver(depth);
}
