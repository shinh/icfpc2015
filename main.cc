#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "game.h"
#include "problem.h"
#include "solver.h"

struct SolverResult {
  unique_ptr<SolverBase> solver;
  string commands;
  int score;
  int seed;
};

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
queue<SolverResult*> g_tasks;
int g_num_running;
int g_finish_pipe[2];

void ReportResult(const string& commands, int score, SolverResult* result) {
  pthread_mutex_lock(&g_mutex);
  result->commands = commands;
  result->score = score;
  pthread_mutex_unlock(&g_mutex);
}

void* WorkerThread(void*) {
  while (true) {
    pthread_mutex_lock(&g_mutex);
    if (g_tasks.empty()) {
      g_num_running--;
      fprintf(stderr, "worker thread finishes rest=%d\n", g_num_running);
      if (g_num_running == 0) {
        char c = 42;
        write(g_finish_pipe[1], &c, 1);
      }
      pthread_mutex_unlock(&g_mutex);
      break;
    }
    SolverResult* result = g_tasks.front();
    g_tasks.pop();
    pthread_mutex_unlock(&g_mutex);

    result->solver->Play();
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  Lcg::Test();
  Pos::Test();
  Decision::Test();

  vector<string> phrases;

  int time_limit = -1;
  int mem_limit = -1;
  int num_cores = 8;

  const char* filename = "problems/problem_0.json";
  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if (!strcmp(arg, "-f")) {
      filename = argv[++i];
    } else if (!strcmp(arg, "-p")) {
      phrases.push_back(argv[++i]);
    } else if (!strcmp(arg, "-c")) {
      num_cores = atoi(argv[++i]);
    } else if (!strcmp(arg, "-t")) {
      time_limit = atoi(argv[++i]);
    } else if (!strcmp(arg, "-m")) {
      mem_limit = atoi(argv[++i]);
    }
  }

  if (phrases.empty()) {
    // Known phrases.
    phrases.push_back("ia! ia!");
    phrases.push_back("ei!");
    phrases.push_back("r'lyeh");
    phrases.push_back("yuggoth");
  }

  vector<SolverResult*> srs;

  Problem problem(filename);
  //vector<pair<int, string>> solutions;
  int game_index = 0;
  for (int seed : problem.source_seeds) {
    SolverResult* sr = new SolverResult;
    SolverBase* solver = MakeNaiveSolver();
    solver->Init(problem, seed, game_index++, phrases, sr, ReportResult);
    sr->solver.reset(solver);
    sr->score = -1;
    sr->seed = seed;
    srs.push_back(sr);
    g_tasks.push(sr);
  }

  pipe(g_finish_pipe);

  vector<pthread_t> threads;
  for (int i = 0; i < num_cores; i++) {
    g_num_running++;
    pthread_t th;
    pthread_create(&th, NULL, WorkerThread, NULL);
  }

  struct timeval timeout;
  timeout.tv_sec = time_limit - 1;
  // Keep 0.3 secs.
  timeout.tv_usec = 1000 * 700;
  struct timeval* timeoutp = NULL;
  if (time_limit >= 1)
    timeoutp = &timeout;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(g_finish_pipe[0], &fds);
  select(g_finish_pipe[0] + 1, &fds, NULL, NULL, timeoutp);

  map<int, SolverResult*> solutions;
  // So all other threads stop writing their results.
  pthread_mutex_lock(&g_mutex);
  for (SolverResult* sr : srs) {
    auto p = solutions.emplace(sr->seed, sr);
    if (!p.second) {
      SolverResult* psr = p.first->second;
      if (psr->score < sr->score) {
        p.first->second = sr;
      }
    }
  }

  printf("[");
  bool is_first = true;
  for (const auto& p : solutions) {
    if (!is_first)
      printf(",");
    is_first = false;
    SolverResult* sr = p.second;

    fprintf(stderr, "seed=%d score=%d\n", sr->seed, sr->score);

    printf("{\"problemId\":%d,\"seed\":%d,\"solution\":\"%s\"}",
           problem.id, p.first, sr->commands.c_str());
  }
  printf("]");
}
