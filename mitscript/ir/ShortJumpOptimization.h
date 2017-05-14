#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

// over-constrain since 1 IR instruction could generate multiple ASM
#define SHORT_JUMP_MAX 1

namespace IR {
  class ShortJumpOptimization : public Optimization {
    using Optimization::Optimization;

  public:
    virtual void optimize() {
      size_t count = 0;
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Jump: {
            auto jump = dynamic_cast<Jump*>(instruction);
            if (abs(count - compiler.bytecode->labels[jump->label->num]) <= SHORT_JUMP_MAX) {
              compiler.instructions[count] = new ShortJump{jump->label};
              delete(jump);
            }
            break;
          }
        }
        count++;
      }
    }
  };
}
