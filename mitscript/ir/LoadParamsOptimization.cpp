#include "LoadParamsOptimization.h"

using namespace std;

namespace IR {
  void LoadParamsOptimization::optimize() {
    InstructionList newIr;
    newIr.reserve(compiler.instructions.size() + compiler.bytecode->parameter_count_);
    for (size_t i = 0; i < compiler.bytecode->parameter_count_; ++i) {
      if (compiler.vars.count(i)) {
        compiler.vars[i]->live_start = 0;
        newIr.push_back(new ForceLoad<Var>{compiler.vars[i]});
      }
    }
    newIr.insert(newIr.end(), compiler.instructions.begin(), compiler.instructions.end());
    compiler.instructions.swap(newIr);
  }
}
