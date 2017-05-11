#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class PropagateTypesOptimization : public Optimization {
  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              assign->dest->transferHint(assign->src);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              assign->dest->transferHint(assign->src);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              assign->dest->transferHint(assign->src);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              assign->dest->transferHint(assign->src);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              assign->dest->transferHint(assign->src);
            }
            break;
          }
         case IR::Operation::Store: {
          if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
            store->src->hintVar(store->dest->num);
            store->dest->transferHint(store->src);
          } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
            store->dest->transferHint(store->src);
          } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
            store->dest->transferHint(store->src);
          }
          break;
        }
        }
      }
    }
  };
}
