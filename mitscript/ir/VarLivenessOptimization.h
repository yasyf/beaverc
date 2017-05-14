#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class VarLivenessOptimization : public Optimization {
    size_t count = 0;

    using Optimization::Optimization;

    void read(shared_ptr<Var> op) {
      if (op->live_start == 0) {
        op->live_start = count;
      }
      op->live_end = count;
    }

    void adjust_live_ends(size_t label_num) {
      size_t instno = compiler.bytecode->labels[label_num];
      for (size_t i = instno; i <= count; ++i) {
        auto instruction = compiler.instructions[i];
        if (instruction->op() == IR::Operation::Assign) {
          if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
            if (assign->src->live_end <= count) {
              assign->src->live_end = count;
            }
          }
        }
      }
    }

  public:
    virtual void optimize() {
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              read(assign->src);
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
  };
}
