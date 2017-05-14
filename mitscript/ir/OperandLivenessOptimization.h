#pragma once
#include "Instructions.h"
#include "Optimization.h"
#include <typeinfo>

using namespace std;

namespace IR {
  class OperandLivenessOptimization : public Optimization {
    size_t count = 0;

    using Optimization::Optimization;

    void alive(shared_ptr<Operand> op) {
      if (op->live_start == 0) {
        op->live_start = count;
      }
    }

    void dead(shared_ptr<Operand> op) {
      op->live_end = count;
    }

  public:
    virtual void optimize() {
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              alive(assign->src);
              alive(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              alive(assign->src);
              alive(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              alive(assign->src);
              alive(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              alive(assign->src);
              alive(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              alive(assign->src);
              alive(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              alive(assign->src);
              alive(assign->dest);
            }
            break;
          }
          case IR::Operation::Store: {
            if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
              dead(store->src);
              dead(store->dest);
            } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
              dead(store->src);
              dead(store->dest);
            } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
              dead(store->src);
              dead(store->dest);
            }
            break;
          }
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            dead(add->src1);
            dead(add->src2);
            alive(add->dest);
            break;
          }
          case IR::Operation::IntAdd: {
            auto intadd = dynamic_cast<IntAdd*>(instruction);
            dead(intadd->src1);
            dead(intadd->src2);
            alive(intadd->dest);
            break;
          }
          case IR::Operation::Sub: {
            auto sub = dynamic_cast<Sub*>(instruction);
            dead(sub->src1);
            dead(sub->src2);
            alive(sub->dest);
            break;
          }
          case IR::Operation::Mul: {
            auto mul = dynamic_cast<Mul*>(instruction);
            dead(mul->src1);
            dead(mul->src2);
            alive(mul->dest);
            break;
          }
          case IR::Operation::Div: {
            auto div = dynamic_cast<Div*>(instruction);
            dead(div->src1);
            dead(div->src2);
            alive(div->dest);
            break;
          }
          case IR::Operation::Gt: {
            auto gt = dynamic_cast<Gt*>(instruction);
            dead(gt->src1);
            dead(gt->src2);
            alive(gt->dest);
            break;
          }
          case IR::Operation::Geq: {
            auto gte = dynamic_cast<Geq*>(instruction);
            dead(gte->src1);
            dead(gte->src2);
            alive(gte->dest);
            break;
          }
          case IR::Operation::Eq: {
            auto eq = dynamic_cast<Eq*>(instruction);
            dead(eq->src1);
            dead(eq->src2);
            alive(eq->dest);
            break;
          }
          case IR::Operation::Neg: {
            auto neg = dynamic_cast<Neg*>(instruction);
            dead(neg->src);
            alive(neg->dest);
            break;
          }
          case IR::Operation::Not: {
            auto nott = dynamic_cast<Not*>(instruction);
            dead(nott->src);
            alive(nott->dest);
            break;
          }
          case IR::Operation::CondJump: {
            auto cjump = dynamic_cast<CondJump*>(instruction);
            dead(cjump->cond);
            break;
          }
          case IR::Operation::Call: {
            auto call = dynamic_cast<IR::Call*>(instruction);
            for (int i = call->args.size() - 1; i >= 0; --i) {
              dead(call->args[i]);
            }
            dead(call->closure);
            break;
          }
          case IR::Operation::Return: {
            auto ret = dynamic_cast<IR::Return*>(instruction);
            dead(ret->val);
            break;
          }
          case IR::Operation::CallHelper: {
            if (auto op = dynamic_cast<CallHelper<Helper::FieldLoad>*>(instruction)) {
              dead(op->args[0]);
            } else if (auto op = dynamic_cast<CallHelper<Helper::FieldStore>*>(instruction)) {
              dead(op->args[0]);
              dead(op->args[1]);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexLoad>*>(instruction)) {
              dead(op->args[0]);
              dead(op->args[1]);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexStore>*>(instruction)) {
              dead(op->args[0]);
              dead(op->args[1]);
              dead(op->args[2]);
            }
            break;
          }
          case IR::Operation::CallAssert: {
            if (auto op = dynamic_cast<CallAssert<Assert::AssertInt>*>(instruction)) {
              dead(op->arg);
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertNotZero>*>(instruction)) {
              dead(op->arg);
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertBool>*>(instruction)) {
              dead(op->arg);
            }
            break;
          }
          case IR::Operation::AllocClosure: {
            auto op = dynamic_cast<AllocClosure*>(instruction);
            dead(op->function);
            for (shared_ptr<Temp> t : op->refs) {
              dead(t);
            }
            break;
          }
          case IR::Operation::And: {
            auto andd = dynamic_cast<And*>(instruction);
            dead(andd->src1);
            dead(andd->src2);
            alive(andd->dest);
            break;
          }
          case IR::Operation::Or: {
            auto orr = dynamic_cast<Or*>(instruction);
            dead(orr->src1);
            dead(orr->src2);
            alive(orr->dest);
            break;
          }
          case IR::Operation::Fork: {
            auto fork = dynamic_cast<Fork*>(instruction);
            dead(fork->src);
            dead(fork->dest1);
            alive(fork->dest2);
            break;
          }
        }
        count++;
      }
    }
  };
}
