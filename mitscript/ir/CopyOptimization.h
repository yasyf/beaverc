#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class CopyOptimization : public Optimization {
    template<typename T>
    bool maybe_replace_copy(InstructionList& ir, size_t count, Instruction* instruction) {
      auto store = dynamic_cast<Store<T>*>(ir[count-1]);
      auto assign = dynamic_cast<Assign<T>*>(instruction);

      if (assign && store && assign->src->num == store->dest->num) {
        ir[count] = new Assign<Temp>{assign->dest, store->src};
        delete(assign);
        return true;
      }

      return false;
    }

  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      size_t count = 0;
      for (auto instruction : ir) {
        if (count == 0) {
          count++;
          continue;
        }

        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (maybe_replace_copy<Var>(ir, count, instruction)) {
              break;
            }
            if (maybe_replace_copy<Deref>(ir, count, instruction)) {
              break;
            }
            if (maybe_replace_copy<Glob>(ir, count, instruction)) {
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
