#pragma once
#include "../bccompiler/Instructions.h"
#include "Instructions.h"
#include "Exception.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <stack>
#include <map>

using namespace std;

namespace IR {
  class Compiler {
    stack<shared_ptr<Temp>> operands;
    shared_ptr<BC::Function> bytecode;
    InstructionList& instructions;
    size_t temp_count = 0;

    shared_ptr<Temp> popTemp() {
      shared_ptr<Temp> t = operands.top();
      operands.pop();
      return t;
    }

    void swapTemp() {
      shared_ptr<Temp> t1 = operands.top();
      operands.pop();
      shared_ptr<Temp> t2 = operands.top();
      operands.pop();

      operands.push(t1);
      operands.push(t2);
    }

    shared_ptr<Temp> peekTemp() {
      return operands.top();
    }

    shared_ptr<Temp> nextTemp() {
      shared_ptr<Temp> t(new Temp{temp_count++});
      operands.push(t);
      return t;
    }

    template<typename T>
    shared_ptr<T> get_operand(size_t num) {
      static map<size_t, shared_ptr<T>> operands;

      if (!operands.count(num)) {
        operands[num] = make_shared<T>(num);
      }
      return operands[num];
    }

    template<typename T>
    shared_ptr<T> get_singleton() {
      static shared_ptr<T> instance = nullptr;

      if (!instance)
        instance = make_shared<T>();

      return instance;
    }

    template<typename T>
    void assign(shared_ptr<T> t) {
      instructions.push_back(new Assign<T>{nextTemp(), t});
    }

    template<typename T>
    void store(shared_ptr<T> t) {
      instructions.push_back(new Store<T>{t, popTemp()});
    }

    template<typename T, Assert A>
    T* helper_binop() {
      auto arg1 = popTemp();
      auto arg2 = popTemp();
      instructions.push_back(new CallAssert<A>{arg1});
      instructions.push_back(new CallAssert<A>{arg2});
      T* op = new T{nextTemp(), arg1, arg2};
      instructions.push_back(op);
      return op;
    }

    template<typename T, Assert A>
    T* helper_unop() {
      auto arg = popTemp();
      instructions.push_back(new CallAssert<A>{arg});
      T* op = new T{nextTemp(), arg};
      instructions.push_back(op);
      return op;
    }

    template<typename T>
    void int_binop() {
      auto op = helper_binop<T, Assert::AssertInt>();
      op->dest->hintInt();
    }

    template<typename T>
    void bool_binop() {
      auto op = helper_binop<T, Assert::AssertBool>();
      op->dest->hintBool();
    }

    template<typename T>
    void int_unop() {
      auto op = helper_unop<T, Assert::AssertInt>();
      op->dest->hintInt();
    }

    template<typename T>
    void bool_unop() {
      auto op = helper_unop<T, Assert::AssertBool>();
      op->dest->hintBool();
    }

    void compile(BC::Function& func) {
      for (auto instruction : func.instructions) {
        switch(instruction.operation) {
          case BC::Operation::Call: {
            size_t arg_count = instruction.operand0.value();
            auto closure = popTemp();
            vector<shared_ptr<Temp>> args;
            for (size_t i = 0; i < arg_count; ++i)
              args.push_back(popTemp());
            reverse(args.begin(), args.end());
            instructions.push_back(new Call{closure, args});
            assign(get_singleton<RetVal>());
            break;
          }
          case BC::Operation::LoadReference:
            assign(get_operand<Deref>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::PushReference:
            assign(get_operand<Ref>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::LoadFunc:
            assign(get_operand<Function>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::LoadGlobal:
            assign(get_operand<Glob>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::LoadLocal:
            assign(get_operand<Var>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::LoadConst:
            assign(make_shared<Const>(func.constants_[instruction.operand0.value()]));
            break;
          case BC::Operation::StoreReference:
            store(get_operand<Deref>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::StoreLocal:
            store(get_operand<Var>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::StoreGlobal:
            store(get_operand<Glob>((size_t)instruction.operand0.value()));
            break;
          case BC::Operation::Eq: {
            auto arg1 = popTemp();
            auto arg2 = popTemp();
            instructions.push_back(new Eq{nextTemp(), arg2, arg1});
            break;
          }
          case BC::Operation::Add: {
            auto arg1 = popTemp();
            auto arg2 = popTemp();
            instructions.push_back(new Add{nextTemp(), arg2, arg1});
            break;
          }
          case BC::Operation::Sub:
            int_binop<Sub>();
            break;
          case BC::Operation::Mul:
            int_binop<Mul>();
            break;
          case BC::Operation::Div: {
            auto arg1 = popTemp();
            auto arg2 = popTemp();
            instructions.push_back(new CallAssert<Assert::AssertInt>{arg1});
            instructions.push_back(new CallAssert<Assert::AssertInt>{arg2});
            instructions.push_back(new CallAssert<Assert::AssertNotZero>{arg1});
            instructions.push_back(new Div{nextTemp(), arg2, arg1});
            break;
          }
          case BC::Operation::Gt:
            int_binop<Gt>();
            break;
          case BC::Operation::Geq:
            int_binop<Geq>();
            break;
          case BC::Operation::And:
            bool_binop<And>();
            break;
          case BC::Operation::Or:
            bool_binop<Or>();
            break;
          case BC::Operation::Neg:
            int_unop<Neg>();
            break;
          case BC::Operation::Not:
            bool_unop<Not>();
            break;
          case BC::Operation::Return:
            instructions.push_back(new Return{popTemp()});
            break;
          case BC::Operation::GarbageCollect:
            instructions.push_back(new CallHelper<Helper::GarbageCollect>{});
            break;
          case BC::Operation::AllocRecord:
            instructions.push_back(new CallHelper<Helper::AllocRecord>{});
            assign(get_singleton<RetVal>());
            break;
          case BC::Operation::FieldLoad:
            instructions.push_back(new CallHelper<Helper::FieldLoad>{(size_t)instruction.operand0.value(), popTemp()});
            assign(get_singleton<RetVal>());
            break;
          case BC::Operation::FieldStore: {
            auto value = popTemp();
            auto record = popTemp();
            instructions.push_back(new CallHelper<Helper::FieldStore>{(size_t)instruction.operand0.value(), record, value});
            break;
          }
          case BC::Operation::IndexLoad: {
            auto index = popTemp();
            auto record = popTemp();
            instructions.push_back(new CallHelper<Helper::IndexLoad>{record, index});
            assign(get_singleton<RetVal>());
            break;
          }
          case BC::Operation::IndexStore: {
            auto value = popTemp();
            auto index = popTemp();
            auto record = popTemp();
            instructions.push_back(new CallHelper<Helper::IndexStore>{record, index, value});
            break;
          }
          case BC::Operation::AllocClosure: {
            size_t ref_count = instruction.operand0.value();
            auto function = popTemp();
            vector<shared_ptr<Temp>> refs;
            for (size_t i = 0; i < ref_count; ++i) {
              refs.push_back(popTemp());
            }
            instructions.push_back(new AllocClosure{function, refs});
            assign(get_singleton<RetVal>());
            break;
          }
          case BC::Operation::Label: {
            shared_ptr<Label> label(new Label{instruction.operand0.value()});
            instructions.push_back(new OutputLabel{label});
            break;
          }
          case BC::Operation::Goto: {
            shared_ptr<Label> label(new Label{instruction.operand0.value()});
            instructions.push_back(new Jump{label});
            break;
          }
          case BC::Operation::If: {
            shared_ptr<Label> label(new Label{instruction.operand0.value()});
            auto cond = popTemp();
            instructions.push_back(new CallAssert<Assert::AssertBool>{cond});
            instructions.push_back(new CondJump{cond, label});
            break;
          }
          case BC::Operation::Dup:
            assign(peekTemp());
            break;
          case BC::Operation::Swap:
            swapTemp();
            break;
          case BC::Operation::Pop:
            popTemp();
            break;
          default:
            throw InvalidOperationException(to_string(static_cast<int>(instruction.operation)));
            break;
        }
      }
    };

  public:
    Compiler(shared_ptr<BC::Function> bytecode, InstructionList& instructions)
      : bytecode(bytecode), instructions(instructions)
      {}

    size_t compile() {
      compile(*bytecode);
      return temp_count;
    }
  };
}
