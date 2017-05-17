#include "VarLivenessOptimization.h"

using namespace std;

namespace IR {
  void VarLivenessOptimization::write(shared_ptr<Var> var) {
    if (var->live_start == -1 || var->live_start > count) {
      var->live_start = count;
    }
    if (var->live_end == INT_MAX) {
      var->live_end = count;
    }
  }

  void VarLivenessOptimization::read(shared_ptr<Var> var) {
    if (var->live_start == -1 || var->live_start > count) {
      var->live_start = count;
    }
    var->live_end = count;
  }

  void VarLivenessOptimization::adjust_live_ends(size_t label_num) {
    size_t instno = compiler.bytecode->labels[label_num];
    for (size_t i = instno; i <= count; ++i) {
      auto instruction = compiler.instructions[i];
      if (instruction->op() == IR::Operation::Assign) {
        if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
          if (assign->src->live_end < count) {
            assign->src->live_end = count;
          }
        }
      }
    }
  }

  void VarLivenessOptimization::optimize() {
    for (auto instruction : compiler.instructions) {
      switch (instruction->op()) {
        case IR::Operation::Assign: {
          if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
            read(assign->src);
          }
          break;
        }
        case IR::Operation::Store: {
          if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
            write(store->dest);
          }
          break;
        }
        case IR::Operation::Jump: {
          auto jump = dynamic_cast<Jump*>(instruction);
          adjust_live_ends(jump->label->num);
          break;
        }
        case IR::Operation::CondJump: {
          auto cjump = dynamic_cast<CondJump*>(instruction);
          adjust_live_ends(cjump->label->num);
          break;
        }
      }
      count++;
    }
  }
}
