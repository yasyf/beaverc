#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class RegisterAllocationOptimization : public Optimization {
  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {

    }
  };
}
