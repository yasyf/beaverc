#pragma once
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class DeadTempOptimization : public Optimization {
    using Optimization::Optimization;

    size_t count = 0;
    map<size_t, map<size_t, shared_ptr<Temp>>> instno_aliases;

    bool maybe_obsolete_temp(shared_ptr<Temp> temp) {
      if (temp->live_end == INT_MAX) {
        obsolete.insert(temp->num);
        return true;
      }
      return false;
    }

    void replace_temp_temp_assignment(Assign<Temp>* assign) {
      if (assign->dest->live_end == INT_MAX) {
        return;
      }
      instno_aliases[assign->dest->live_end][assign->dest->num] = assign->src;
    }

    void maybe_resolve_alias(shared_ptr<Temp>* temp) {
      if (instno_aliases.count(count)) {
        auto num = (*temp)->num;
        if (instno_aliases[count].count(num)) {
          obsolete.insert(num);
          *temp = instno_aliases[count][num];
        }
      }
    }

  public:
    virtual void optimize() {
      for (auto instruction : compiler.instructions) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              if (!maybe_obsolete_temp(assign->dest)) {
                replace_temp_temp_assignment(assign);
              }
            } else if (auto assign = dynamic_cast<Assign<RetVal>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              maybe_obsolete_temp(assign->dest);
            }
            break;
          }
          case IR::Operation::Store: {
            if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
              maybe_resolve_alias(&store->src);
            } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
              maybe_resolve_alias(&store->src);
            } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
              maybe_resolve_alias(&store->src);
            }
            break;
          }
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            maybe_resolve_alias(&add->src1);
            maybe_resolve_alias(&add->src2);
            maybe_obsolete_temp(add->dest);
            break;
          }
          case IR::Operation::IntAdd: {
            auto intadd = dynamic_cast<IntAdd*>(instruction);
            maybe_resolve_alias(&intadd->src1);
            maybe_resolve_alias(&intadd->src2);
            maybe_obsolete_temp(intadd->dest);
            break;
          }
          case IR::Operation::Sub: {
            auto sub = dynamic_cast<Sub*>(instruction);
            maybe_resolve_alias(&sub->src1);
            maybe_resolve_alias(&sub->src2);
            maybe_obsolete_temp(sub->dest);
            break;
          }
          case IR::Operation::Mul: {
            auto mul = dynamic_cast<Mul*>(instruction);
            maybe_resolve_alias(&mul->src1);
            maybe_resolve_alias(&mul->src2);
            maybe_obsolete_temp(mul->dest);
            break;
          }
          case IR::Operation::Div: {
            auto div = dynamic_cast<Div*>(instruction);
            maybe_resolve_alias(&div->src1);
            maybe_resolve_alias(&div->src2);
            maybe_obsolete_temp(div->dest);
            break;
          }
          case IR::Operation::Gt: {
            auto gt = dynamic_cast<Gt*>(instruction);
            maybe_resolve_alias(&gt->src1);
            maybe_resolve_alias(&gt->src2);
            maybe_obsolete_temp(gt->dest);
            break;
          }
          case IR::Operation::Geq: {
            auto gte = dynamic_cast<Geq*>(instruction);
            maybe_resolve_alias(&gte->src1);
            maybe_resolve_alias(&gte->src2);
            maybe_obsolete_temp(gte->dest);
            break;
          }
          case IR::Operation::Eq: {
            auto eq = dynamic_cast<Eq*>(instruction);
            maybe_resolve_alias(&eq->src1);
            maybe_resolve_alias(&eq->src2);
            maybe_obsolete_temp(eq->dest);
            break;
          }
          case IR::Operation::FastEq: {
            auto feq = dynamic_cast<FastEq*>(instruction);
            maybe_resolve_alias(&feq->src1);
            maybe_resolve_alias(&feq->src2);
            maybe_obsolete_temp(feq->dest);
            break;
          }
          case IR::Operation::Neg: {
            auto neg = dynamic_cast<Neg*>(instruction);
            maybe_resolve_alias(&neg->src);
            maybe_obsolete_temp(neg->dest);
            break;
          }
          case IR::Operation::Not: {
            auto nott = dynamic_cast<Not*>(instruction);
            maybe_resolve_alias(&nott->src);
            maybe_obsolete_temp(nott->dest);
            break;
          }
          case IR::Operation::CondJump: {
            auto cjump = dynamic_cast<CondJump*>(instruction);
            maybe_resolve_alias(&cjump->cond);
            break;
          }
          case IR::Operation::Call: {
            auto call = dynamic_cast<IR::Call*>(instruction);
            for (int i = call->args.size() - 1; i >= 0; --i) {
              maybe_resolve_alias(&call->args[i]);
            }
            maybe_resolve_alias(&call->closure);
            break;
          }
          case IR::Operation::Return: {
            auto ret = dynamic_cast<IR::Return*>(instruction);
            maybe_resolve_alias(&ret->val);
            break;
          }
          case IR::Operation::CallHelper: {
            if (auto op = dynamic_cast<CallHelper<Helper::FieldLoad>*>(instruction)) {
              maybe_resolve_alias(&op->args[0]);
            } else if (auto op = dynamic_cast<CallHelper<Helper::FieldStore>*>(instruction)) {
              maybe_resolve_alias(&op->args[0]);
              maybe_resolve_alias(&op->args[1]);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexLoad>*>(instruction)) {
              maybe_resolve_alias(&op->args[0]);
              maybe_resolve_alias(&op->args[1]);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexStore>*>(instruction)) {
              maybe_resolve_alias(&op->args[0]);
              maybe_resolve_alias(&op->args[1]);
              maybe_resolve_alias(&op->args[2]);
            }
            break;
          }
          case IR::Operation::CallAssert: {
            if (auto op = dynamic_cast<CallAssert<Assert::AssertInt>*>(instruction)) {
              maybe_resolve_alias(&op->arg);
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertNotZero>*>(instruction)) {
              maybe_resolve_alias(&op->arg);
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertBool>*>(instruction)) {
              maybe_resolve_alias(&op->arg);
            }
            break;
          }
          case IR::Operation::AllocClosure: {
            auto op = dynamic_cast<AllocClosure*>(instruction);
            maybe_resolve_alias(&op->function);
            for (shared_ptr<Temp> t : op->refs) {
              maybe_resolve_alias(&t);
            }
            break;
          }
          case IR::Operation::And: {
            auto andd = dynamic_cast<And*>(instruction);
            maybe_resolve_alias(&andd->src1);
            maybe_resolve_alias(&andd->src2);
            maybe_obsolete_temp(andd->dest);
            break;
          }
          case IR::Operation::Or: {
            auto orr = dynamic_cast<Or*>(instruction);
            maybe_resolve_alias(&orr->src1);
            maybe_resolve_alias(&orr->src2);
            maybe_obsolete_temp(orr->dest);
            break;
          }
          case IR::Operation::Fork: {
            auto fork = dynamic_cast<Fork*>(instruction);
            maybe_resolve_alias(&fork->src);
            maybe_obsolete_temp(fork->dest1);
            maybe_obsolete_temp(fork->dest2);
            break;
          }
        }
        count++;
      }
    }
  };
}
