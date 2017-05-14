#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class RemoveObsoleteOptimization : public Optimization {
    set<size_t>& obsolete;

    void delete_inst(size_t count, Instruction* inst) {
      compiler.instructions[count] = Noop::Singleton();
      delete(inst);
    }

    bool delete_if_obsolete(size_t count, shared_ptr<Temp> arg, Instruction* inst) {
      if (obsolete.count(arg->num)) {
        delete_inst(count, inst);
        return true;
      }
      return false;
    }

  public:
    RemoveObsoleteOptimization(Compiler& compiler, set<size_t>& obsolete) : Optimization(compiler), obsolete(obsolete) {}

    virtual void optimize() {
      size_t count = 0;
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<RetVal>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              delete_if_obsolete(count, assign->dest, assign);
            }
            break;
          }
          case IR::Operation::CallAssert: {
            if (auto op = dynamic_cast<CallAssert<Assert::AssertInt>*>(instruction)) {
              if (!delete_if_obsolete(count, op->arg, op) && op->arg->isInt()) {
                delete_inst(count, op);
              }
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertNotZero>*>(instruction)) {
              if (!delete_if_obsolete(count, op->arg, op) && op->arg->isInt() && op->arg->isConst()) {
                int64_t i = op->arg->getConst().getInteger();
                if (i != 0) {
                  delete_inst(count, op);
                }
              }
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertBool>*>(instruction)) {
              if (!delete_if_obsolete(count, op->arg, op) && op->arg->isBool()) {
                delete_inst(count, op);
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
