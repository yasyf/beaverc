#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class PropagateTypesOptimization : Optimization {
  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              assign->dest->type_hint = assign->src->type_hint;
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              assign->dest->type_hint = assign->src->type_hint;
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              assign->dest->type_hint = assign->src->type_hint;
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              assign->dest->type_hint = assign->src->type_hint;
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              assign->dest->type_hint = assign->src->type_hint;
            }
            break;
          }
         case IR::Operation::Store: {
          if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
            store->dest->type_hint = store->src->type_hint;
          } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
            store->dest->type_hint = store->src->type_hint;
          } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
            store->dest->type_hint = store->src->type_hint;
          }
          break;
        }
        }
      }
    }
  };
}
