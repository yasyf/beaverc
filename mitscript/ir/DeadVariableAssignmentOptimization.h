#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class DeadVariableAssignmentOptimization : public Optimization {
    using Optimization::Optimization;

  public:
    size_t count = 0;

    virtual void optimize() {
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Store: {
            if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
              if (store->dest->live_end <= count) {
                obsolete.insert(store->src->num);
                compiler.instructions[count] = Noop::Singleton();
                delete(instruction);
              }
            }
            break;
          }
        }
        count++;
      }
    }
  };
}
