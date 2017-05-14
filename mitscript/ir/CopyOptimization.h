#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class CopyOptimization : public Optimization {
    size_t count = 0;
    InstructionList newIr;

    using Optimization::Optimization;

    template<typename T>
    bool maybe_replace_copy(Instruction* instruction) {
      auto store = dynamic_cast<Store<T>*>(newIr[count-1]);
      auto assign = dynamic_cast<Assign<T>*>(instruction);

      if (assign && store && assign->src->num == store->dest->num) {
        auto f1 = compiler.extraTemp();
        auto f2 = compiler.extraTemp();

        auto fork = new Fork{store->src, f1, f2};

        store->src = f1;
        newIr[count] = new Assign<Temp>{assign->dest, f2};
        delete(assign);
        newIr.insert(newIr.begin() + (count-1), fork);
        count++;
        return true;
      }

      return false;
    }

  public:
    virtual void optimize() {
      newIr.reserve(compiler.instructions.size());
      for (auto instruction : compiler.instructions) {
        newIr.push_back(instruction);

        if (count == 0) {
          count++;
          continue;
        }

        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (maybe_replace_copy<Var>(instruction)) {
              break;
            }
            if (maybe_replace_copy<Deref>(instruction)) {
              break;
            }
            if (maybe_replace_copy<Glob>(instruction)) {
              break;
            }
            break;
          }
        }
        count++;
      }
      compiler.instructions.swap(newIr);
    }
  };
}
