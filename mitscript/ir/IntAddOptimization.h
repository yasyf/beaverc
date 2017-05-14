#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class IntAddOptimization : public Optimization {
    using Optimization::Optimization;

  public:
    virtual void optimize() {
      size_t count = 0;
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            if (add->src1->isInt() && add->src2->isInt()) {
              add->dest->hintInt();
              compiler.instructions[count] = new IntAdd{add->dest, add->src1, add->src2};
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
