#pragma once
#include "../vm/Value.h"
#include "Exception.h"
#include "Instructions.h"
#include "Optimization.h"

using namespace std;

namespace IR {
  class ConstantFoldingOptimization : public Optimization {
    template<typename T, typename F>
    void foldIntsBinOp(T op, InstructionList& ir, size_t count, F func, bool check = true) {
      if (op->src1->isInt() && op->src2->isInt()) {
        int64_t i1 = op->src1->getConstValue().getInteger();
        int64_t i2 = op->src2->getConstValue().getInteger();
        VM::Value v = VM::Value::makeInteger(func(i1, i2));

        ir[count] = new Assign<Const>{op->dest, make_shared<Const>(v)};

        obsolete.insert(op->src1->num);
        obsolete.insert(op->src2->num);
        delete(op);
      } else if (
        check &&
        (
          (op->src1->hasHint() && !op->src1->canBeInt()) ||
          (op->src2->hasHint() && !op->src2->canBeInt())
        )
      ) {
        throw_exception(IllegalCastException("Value is not an int"));
      }
    }

  public:
    virtual void optimize(shared_ptr<BC::Function> func, InstructionList& ir) {
      size_t count = 0;
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            if (!add->src1->isConst() || !add->src2->isConst()) {
              break;
            }

            if (add->src1->isString() && add->src2->isString()) {
              const char* s1 = add->src1->getConstValue().getStringConstant();
              const char* s2 = add->src2->getConstValue().getStringConstant();
              char* s = new char [strlen(s1)+strlen(s2)+2];
              strcpy(s, s1);
              strcat(s, s2);
              VM::Value v = VM::Value::makeStringConstant(s);

              ir[count] = new Assign<Const>{add->dest, make_shared<Const>(v)};

              obsolete.insert(add->src1->num);
              obsolete.insert(add->src2->num);
              delete[] s1;
              delete[] s2;
              delete(add);
            } else {
              foldIntsBinOp(add, ir, count, [] (int a, int b) { return a + b; }, false);
            }

            break;
          }

          case IR::Operation::IntAdd: {
            auto add = dynamic_cast<IntAdd*>(instruction);
            if (!add->src1->isConst() || !add->src2->isConst()) {
              break;
            }
            foldIntsBinOp(add, ir, count, [] (int a, int b) { return a + b; });
            break;
          }

          case IR::Operation::Sub: {
            auto sub = dynamic_cast<Sub*>(instruction);
            if (!sub->src1->isConst() || !sub->src2->isConst()) {
              break;
            }
            foldIntsBinOp(sub, ir, count, [] (int a, int b) { return b - a; });
            break;
          }

          case IR::Operation::Mul: {
            auto mul = dynamic_cast<Mul*>(instruction);
            if (!mul->src1->isConst() || !mul->src2->isConst()) {
              break;
            }
            foldIntsBinOp(mul, ir, count, [] (int a, int b) { return a * b; });
            break;
          }

          case IR::Operation::Div: {
            auto div = dynamic_cast<Div*>(instruction);
            if (!div->src1->isConst() || !div->src2->isConst()) {
              break;
            }

            int64_t b = div->src2->getConstValue().getInteger();
            if (b == 0) {
              throw_exception(IllegalArithmeticException("divide by zero"));
              break;
            }

            foldIntsBinOp(div, ir, count, [] (int a, int b) { return a / b; });
            break;
          }

          case IR::Operation::Neg: {
            auto neg = dynamic_cast<Neg*>(instruction);
            if (!neg->src->isConst()) {
              break;
            }

            if (!neg->src->isInt()) {
              if (neg->src->hasHint()) {
                throw_exception(IllegalCastException("Value is not an int"));
              }
              break;
            }

            int64_t i = neg->src->getConstValue().getInteger();
            VM::Value v = VM::Value::makeInteger(-i);

            ir[count] = new Assign<Const>{neg->dest, make_shared<Const>(v)};

            obsolete.insert(neg->src->num);
            delete(neg);
            break;
          }

          case IR::Operation::Not: {
            auto nott = dynamic_cast<Not*>(instruction);
            if (!nott->src->isConst()) {
              break;
            }

            if (!nott->src->isBool()) {
              if (nott->src->hasHint()) {
                throw_exception(IllegalCastException("Value is not a bool"));
              }
              break;
            }

            bool b = nott->src->getConstValue().getBoolean();
            VM::Value v = VM::Value::makeBoolean(!b);

            ir[count] = new Assign<Const>{nott->dest, make_shared<Const>(v)};

            obsolete.insert(nott->src->num);
            delete(nott);
            break;
          }
        }
        count++;
      }
    }
  };
}
