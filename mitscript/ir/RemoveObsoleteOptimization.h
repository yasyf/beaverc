#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class RemoveObsoleteOptimization : public Optimization {
    void delete_inst(InstructionList& ir, size_t count, Instruction* inst) {
      ir[count] = Noop::Singleton();
      delete(inst);
    }

    bool delete_if_obsolete(InstructionList& ir, size_t count, shared_ptr<Temp> arg, Instruction* inst) {
      if (obsolete.count(arg->num)) {
        delete_inst(ir, count, inst);
        return true;
      }
      return false;
    }

  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      size_t count = 0;
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<RetVal>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              delete_if_obsolete(ir, count, assign->dest, assign);
            }
            break;
          }
          case IR::Operation::CallAssert: {
            if (auto op = dynamic_cast<CallAssert<Assert::AssertInt>*>(instruction)) {
              if (!delete_if_obsolete(ir, count, op->arg, op) && op->arg->isInt()) {
                delete_inst(ir, count, op);
              }
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertNotZero>*>(instruction)) {
              if (!delete_if_obsolete(ir, count, op->arg, op) && op->arg->isInt() && op->arg->isConst()) {
                int64_t i = op->arg->getConst().getInteger();
                if (i != 0) {
                  delete_inst(ir, count, op);
                }
              }
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertBool>*>(instruction)) {
              if (!delete_if_obsolete(ir, count, op->arg, op) && op->arg->isBool()) {
                delete_inst(ir, count, op);
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
