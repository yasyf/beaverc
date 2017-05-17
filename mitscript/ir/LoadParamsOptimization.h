#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class LoadParamsOptimization : public Optimization {
    using Optimization::Optimization;

  public:
    virtual void optimize() override;
  };
}
