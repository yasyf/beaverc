#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class RemoveNoopOptimization : public Optimization {
    using Optimization::Optimization;

  public:
    virtual void optimize() {
      InstructionList newIr;
      newIr.reserve(compiler.instructions.size());
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Noop: {
            break;
          }
          default: {
            newIr.push_back(instruction);
          }
        }
      }
      compiler.instructions.swap(newIr);
    }
  };
}
