#pragma once
#include "Instructions.h"

using namespace std;

namespace IR {
  class Optimization {
  public:
    void optimize(shared_ptr<BC::Function> bytecode, InstructionList& instructions) = 0;
  };
}
