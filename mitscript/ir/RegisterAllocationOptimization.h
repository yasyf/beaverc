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
  static constexpr std::array<R64, 11> reg_pool = {
    r12, r13, r14, r15,
    r8,  r9,  r10, r11,
    rcx, rdx, rbx,
    // rsi, rdi,
  };

  class RegisterAllocationOptimization : public Optimization {
    struct temps_compare {
        bool operator() (const shared_ptr<Temp> a, const shared_ptr<Temp> b) const {
            return a->live_start < b->live_start;
        }
    };

    struct active_compare {
        bool operator() (const shared_ptr<Temp> a, const shared_ptr<Temp> b) const {
            return a->live_end < b->live_end;
        }
    };

    set<R64> in_use;
    multiset<shared_ptr<Temp>, temps_compare> temps;
    multiset<shared_ptr<Temp>, active_compare> active;

    bool has_free_reg() {
      return in_use.size() < reg_pool.size();
    }

    R64 allocate_reg() {
      for (auto reg : reg_pool) {
        if (!in_use.count(reg)) {
          in_use.insert(reg);
          return reg;
        }
      }
      throw RegistersExhausted("allocate_reg");
    }

    void deallocate_reg(R64& reg) {
      in_use.erase(reg);
    }

    void expire_old_intervals(shared_ptr<Temp> temp) {
      for (auto it = active.begin(); it != active.end(); ) {
        if ((*it)->live_end >= temp->live_start) {
          return;
        }
        deallocate_reg((*it)->reg.value());
        it = active.erase(it);
      }
    }

    void spill_at_interval(shared_ptr<Temp> temp) {
      auto spill = *(active.rbegin());
      if (spill->live_end > temp->live_end) {
        temp->reg = spill->reg;
        spill->reg = experimental::nullopt;
        active.erase(spill);
        active.insert(temp);
      } else {
        temp->reg = experimental::nullopt;
      }
    }

    void linear_scan_allocate() {
      for (auto temp : temps) {
        expire_old_intervals(temp);
        if (!has_free_reg()) {
          spill_at_interval(temp);
        } else {
          temp->reg = allocate_reg();
          active.insert(temp);
        }
      }
    }

  public:
    RegisterAllocationOptimization(Compiler& compiler)
      : Optimization(compiler), temps(compiler.temps.begin(), compiler.temps.end())
    {}

    virtual void optimize() {
      linear_scan_allocate();
    }
  };
}
