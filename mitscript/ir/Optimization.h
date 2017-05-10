#pragma once
#include <set>
#include "Instructions.h"

using namespace std;

namespace IR {
  class Optimization {
  public:
    set<size_t> obsolete;
    virtual void optimize(shared_ptr<BC::Function> bytecode, InstructionList& instructions) = 0;
  };
}
