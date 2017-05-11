#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class RemoveNoopOptimization : public Optimization {
  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      InstructionList newIr;
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Noop: {
            break;
          }
          default: {
            newIr.push_back(instruction);
          }
        }
      }
      ir.swap(newIr);
    }
  };
}
