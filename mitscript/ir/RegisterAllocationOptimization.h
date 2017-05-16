#pragma once
#include "include/x64asm.h"
#include "Instructions.h"
#include "Optimization.h"
#include <set>
#include <vector>
#include <algorithm>

using namespace std;
using namespace x64asm;

namespace IR {
  struct live_start_compare {
    bool operator() (const shared_ptr<Operand> a, const shared_ptr<Operand> b) const {
      return a->live_start < b->live_start;
    }
  };

  struct live_end_compare {
    bool operator() (const shared_ptr<Operand> a, const shared_ptr<Operand> b) const {
      return a->live_end < b->live_end;
    }
  };

  class RegisterAllocationOptimization : public Optimization {
    set<R64> in_use;
    multiset<shared_ptr<Operand>, live_start_compare> operands;
    multiset<shared_ptr<Operand>, live_end_compare> active;

    bool has_free_reg();
    R64 allocate_reg(shared_ptr<Operand> operand);
    void deallocate_reg(shared_ptr<Operand> operand);
    void expire_old_intervals(shared_ptr<Operand> operand);
    void spill_at_interval(shared_ptr<Operand> operand);
    void linear_scan_allocate();

  public:
    RegisterAllocationOptimization(Compiler& compiler);
    virtual void optimize() override;
  };
}
