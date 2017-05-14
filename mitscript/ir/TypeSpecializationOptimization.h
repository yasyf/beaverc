#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class TypeSpecializationOptimization : public Optimization {
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
          case IR::Operation::Eq: {
            auto eq = dynamic_cast<Eq*>(instruction);
            if (
              (eq->src1->isInt() && eq->src2->isInt()) ||
              (eq->src1->isBool() && eq->src2->isBool())
            ) {
              compiler.instructions[count] = new FastEq{eq->dest, eq->src1, eq->src2};
              delete(eq);
            }
            break;
          }
        }
        count++;
      }
    }
  };
}
