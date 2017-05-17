#include "RegisterAllocationOptimization.h"

using namespace std;
using namespace x64asm;

namespace IR {
  static constexpr std::array<R64, 9> reg_pool = {
    r12, r13, r14, r15,
    r8,  r9,  r10, r11,
    rbx,
  };

  bool RegisterAllocationOptimization::has_free_reg() {
    return in_use.size() < reg_pool.size();
  }

  R64 RegisterAllocationOptimization::allocate_reg(shared_ptr<Operand> operand) {
    if (auto var = dynamic_pointer_cast<Var>(operand)) {
      if (var->last_reg && !in_use.count(var->last_reg.value())) {
        in_use.insert(var->last_reg.value());
        return var->last_reg.value();
      }
    }

    for (auto reg : reg_pool) {
      if (!in_use.count(reg)) {
        in_use.insert(reg);
        return reg;
      }
    }
    throw RegistersExhausted("allocate_reg");
  }

  void RegisterAllocationOptimization::deallocate_reg(shared_ptr<Operand> operand) {
    in_use.erase(operand->reg.value());
    if (auto temp = dynamic_pointer_cast<Temp>(operand)) {
      if (temp->isVar()) {
        temp->shared_reg = true;
        compiler.vars[temp->getVar()]->last_reg = temp->reg;
      }
    }
  }

  void RegisterAllocationOptimization::expire_old_intervals(shared_ptr<Operand> operand) {
    for (auto it = active.begin(); it != active.end(); ) {
      if ((*it)->live_end > operand->live_start) {
        return;
      }
      deallocate_reg(*it);
      it = active.erase(it);
    }
  }

  void RegisterAllocationOptimization::spill_at_interval(shared_ptr<Operand> operand) {
    auto spill = *(active.rbegin());
    if (spill->live_end > operand->live_end) {
      operand->reg = spill->reg;
      spill->reg = experimental::nullopt;
      active.erase(spill);
      active.insert(operand);
    } else {
      operand->reg = experimental::nullopt;
    }
  }

  void RegisterAllocationOptimization::linear_scan_allocate() {
    for (auto operand : operands) {
      expire_old_intervals(operand);
      if (auto temp = dynamic_pointer_cast<Temp>(operand)) {
        if (temp->isVar()) {
          if (auto reg = compiler.vars[temp->getVar()]->reg) {
            temp->reg = reg;
            temp->shared_reg = true;
            continue;
          }
        }
      }
      if (!has_free_reg()) {
        spill_at_interval(operand);
      } else {
        operand->reg = allocate_reg(operand);
        active.insert(operand);
      }
    }
  }

  RegisterAllocationOptimization::RegisterAllocationOptimization(Compiler& compiler) : Optimization(compiler) {
    for (auto const& p : compiler.vars)
      operands.insert(p.second);
    operands.insert(compiler.temps.begin(), compiler.temps.end());
  }

  void RegisterAllocationOptimization::optimize() {
    linear_scan_allocate();
  }
}
