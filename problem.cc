#include "problem.h"

#include <assert.h>

#include <fstream>

#include "picojson.h"

static double GetDouble(picojson::value v) {
  assert(v.is<double>());
  return v.get<double>();
}

static Problem::Pos GetPos(picojson::value v) {
  return Problem::Pos(GetDouble(v.get("x")), GetDouble(v.get("y")));
}

Problem::Problem(const char* filename) {
  picojson::value root;
  fstream ifs(filename);
  ifs >> root;

  height = GetDouble(root.get("height"));
  width = GetDouble(root.get("width"));
  id = GetDouble(root.get("id"));
  source_length = GetDouble(root.get("sourceLength"));

  picojson::value v = root.get("sourceSeeds");
  for (size_t i = 0; v.contains(i); i++) {
    source_seeds.push_back(GetDouble(v.get(i)));
  }

  v = root.get("filled");
  for (size_t i = 0; v.contains(i); i++) {
    filled.push_back(GetPos(v.get(i)));
  }

  v = root.get("units");
  for (size_t i = 0; v.contains(i); i++) {
    picojson::value m = v.get(i).get("members");
    picojson::value p = v.get(i).get("pivot");
    vector<Pos> members;
    for (size_t j = 0; m.contains(j); j++) {
      members.push_back(GetPos(m.get(j)));
    }
    units.push_back(make_pair(members, GetPos(p)));
  }
}
