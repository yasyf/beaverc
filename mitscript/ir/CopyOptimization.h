#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class CopyOptimization : public Optimization {
    using Optimization::Optimization;

    template<typename T>
    bool maybe_replace_copy(size_t count, Instruction* instruction) {
      auto store = dynamic_cast<Store<T>*>(compiler.instructions[count-1]);
      auto assign = dynamic_cast<Assign<T>*>(instruction);

      if (assign && store && assign->src->num == store->dest->num) {
        #warning cannot reuse temp, use fork
        compiler.instructions[count] = new Assign<Temp>{assign->dest, store->src};
        delete(assign);
        return true;
      }

      return false;
    }

  public:
    virtual void optimize() {
      size_t count = 0;
      for (auto instruction : compiler.instructions) {
        if (count == 0) {
          count++;
          continue;
        }

        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (maybe_replace_copy<Var>(count, instruction)) {
              break;
            }
            if (maybe_replace_copy<Deref>(count, instruction)) {
              break;
            }
            if (maybe_replace_copy<Glob>(count, instruction)) {
              break;
            }
            break;
          }
        }
        count++;
      }
    }
  };
}
