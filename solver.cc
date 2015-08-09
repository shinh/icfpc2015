#include "game.h"
#include "problem.h"
#include "solver.h"

void SolverBase::Init(const Problem& problem, int seed, int game_index,
                      const vector<string>& phrases) {
  lcg_.reset(new Lcg(seed));
  phrases_ = phrases;
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

int SolverBase::GetPhraseScore(const string& commands) const {
  int score = 0;
  string cmd = commands;
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

struct MakeNiceCommandCtx {
  Decision d;
  Decision pd;
  vector<Command> cmds;
  int bap_bonus;
};

bool SolverBase::MakeNiceCommandStrFrom(const Unit& u, Decision d,
                                        Decision goal,
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
      MakeNiceCommandCtx nctx;                                          \
      nctx.d = nd;                                                      \
      nctx.pd = d;                                                      \
      nctx.bap_bonus = ctx.bap_bonus;                                   \
      nctx.cmds = cmds;                                                 \
      nctx.cmds.push_back(cmd);                                         \
      int dist = abs(nd.x - goal.x) + abs(nd.y - goal.y) + abs(nd.r - goal.r); \
      q.emplace(dist * 2 + nctx.bap_bonus + prio, nctx);                \
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

bool SolverBase::MakeNiceCommandAfterPhrase(const string& phrase,
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

string SolverBase::MakeNiceCommandStr(const Unit& u, Decision goal) {
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

