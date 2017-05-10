#pragma once
#include "Instructions.h"
#include <stdlib.h>

using namespace std;

namespace IR {
  class IntAddOptimization {
  public:
    size_t optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      size_t count = 0;
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            if (add->src1->isInt() && add->src2->isInt()) {
              add->dest->hintInt();
              ir[count] = new IntAdd{add->dest, add->src1, add->src2};
              delete(add);
            }
            break;
          }
        }
        count++;
      }
    }
  };
}
