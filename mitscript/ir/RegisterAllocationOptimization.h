#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class RegisterAllocationOptimization : public Optimization {
    using Optimization::Optimization;

  public:
    virtual void optimize() {

    }
  };
}
