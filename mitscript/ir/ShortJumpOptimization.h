#pragma once
#include "Instructions.h"
#include <stdlib.h>

using namespace std;

// over-constrain since 1 IR instruction could generate multiple ASM
#define SHORT_JUMP_MAX (1 << 5)

namespace IR {
  class ShortJumpOptimization {
  public:
    size_t optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      size_t count = 0;
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Jump: {
            auto jump = dynamic_cast<Jump*>(instruction);
            if (abs(count - func->labels[jump->label->num]) < SHORT_JUMP_MAX) {
              ir[count] = new ShortJump{jump->label};
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