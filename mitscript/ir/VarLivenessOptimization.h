#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class VarLivenessOptimization : public Optimization {
    size_t count = 0;

    using Optimization::Optimization;

    void write(shared_ptr<Var> var);
    void read(shared_ptr<Var> var);
    void adjust_live_ends(size_t label_num);

  public:
    virtual void optimize() override;
  };
}
