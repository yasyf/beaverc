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
    stack<Temp> operands;
    shared_ptr<BC::Function> bytecode;
    map<size_t, Label> labels;
    InstructionList instructions;
    InstructionList labelledInstructions;
    size_t temp_count = 0;

    Temp popTemp() {
      Temp t = operands.top();
      operands.pop();
      return t;
    }

    void swapTemp() {
      Temp t1 = operands.top();
      operands.pop();
      Temp t2 = operands.top();
      operands.pop();

      operands.push(t1);
      operands.push(t2);
    }

    Temp peekTemp() {
      return operands.top();
    }

    Temp nextTemp() {
      Temp t = Temp{temp_count++};
      operands.push(t);
      return t;
    }

    Label nextLabel() {
      #warning TODO: make this smarter
      return Label{to_string(rand())};
    }

    void addLabel(Label label, size_t delta) {
      size_t num = instructions.size() - 1 + delta;
      labels.emplace(num, label);
    }

    template<typename T>
    void assign(T t) {
      instructions.push_back(new Assign<T>{nextTemp(), t});
    }

    template<typename T>
    void store(T t) {
      instructions.push_back(new Store<T>{t, popTemp()});
    }

    template<typename T, Helper H>
    void helper_binop() {
      Temp arg1 = popTemp();
      Temp arg2 = popTemp();
      instructions.push_back(new CallHelper<H>{arg1});
      instructions.push_back(new CallHelper<H>{arg2});
      instructions.push_back(new T{nextTemp(), arg1, arg2});
    }

    template<typename T, Helper H>
    void helper_unop() {
      Temp arg = popTemp();
      instructions.push_back(new CallHelper<H>{arg});
      instructions.push_back(new T{nextTemp(), arg});
    }

    template<typename T>
    void int_binop() {
      helper_binop<T, Helper::AssertInt>();
    }

    template<typename T>
    void bool_binop() {
      helper_binop<T, Helper::AssertBool>();
    }

    template<typename T>
    void int_unop() {
      helper_unop<T, Helper::AssertInt>();
    }

    template<typename T>
    void bool_unop() {
      helper_unop<T, Helper::AssertBool>();
    }

    void outputLabels() {
      size_t inst_count = 0;
      for (auto inst : instructions) {
        if (labels.count(inst_count)) {
          auto label = labels.find(inst_count);
          labelledInstructions.push_back(new OutputLabel{label->second});
        }
        labelledInstructions.push_back(inst);
        ++inst_count;
      }
    }

    void compile(BC::Function& func) {
      for (auto instruction : func.instructions) {
        switch(instruction.operation) {
          case BC::Operation::Call: {
            size_t arg_count = instruction.operand0.value();
            Temp closure = popTemp();
            vector<Temp> args;
            for (size_t i = 0; i < arg_count; ++i)
              args.push_back(popTemp());
            reverse(args.begin(), args.end());
            instructions.push_back(new Call{closure, args});
            assign(RetVal{});
            break;
          }
          case BC::Operation::LoadReference:
            assign(Deref{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::PushReference:
            assign(Ref{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::LoadFunc:
            assign(Function{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::LoadGlobal:
            assign(Glob{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::LoadLocal:
            assign(Var{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::LoadConst:
            assign(Const{func.constants_[instruction.operand0.value()]});
            break;
          case BC::Operation::StoreReference:
            store(Deref{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::StoreLocal:
            store(Var{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::StoreGlobal:
            store(Glob{(size_t)instruction.operand0.value()});
            break;
          case BC::Operation::Eq: {
            Temp arg1 = popTemp();
            Temp arg2 = popTemp();
            instructions.push_back(new Eq{nextTemp(), arg2, arg1});
            break;
          }
          case BC::Operation::Add: {
            Temp arg1 = popTemp();
            Temp arg2 = popTemp();
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
            Temp arg1 = popTemp();
            Temp arg2 = popTemp();
            instructions.push_back(new CallHelper<Helper::AssertInt>{arg1});
            instructions.push_back(new CallHelper<Helper::AssertInt>{arg2});
            instructions.push_back(new CallHelper<Helper::AssertNotZero>{arg2});
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
            assign(RetVal{});
            break;
          case BC::Operation::FieldLoad:
            instructions.push_back(new CallHelper<Helper::FieldLoad>{(size_t)instruction.operand0.value(), popTemp()});
            assign(RetVal{});
            break;
          case BC::Operation::FieldStore: {
            Temp value = popTemp();
            Temp record = popTemp();
            instructions.push_back(new CallHelper<Helper::FieldStore>{(size_t)instruction.operand0.value(), record, value});
            break;
          }
          case BC::Operation::IndexLoad: {
            Temp index = popTemp();
            Temp record = popTemp();
            instructions.push_back(new CallHelper<Helper::IndexLoad>{record, index});
            assign(RetVal{});
            break;
          }
          case BC::Operation::IndexStore: {
            Temp value = popTemp();
            Temp index = popTemp();
            Temp record = popTemp();
            instructions.push_back(new CallHelper<Helper::IndexStore>{record, index, value});
            break;
          }
          case BC::Operation::AllocClosure: {
            size_t ref_count = instruction.operand0.value();
            Temp function = popTemp();
            vector<Temp> refs;
            for (size_t i = 0; i < ref_count; ++i)
              refs.push_back(popTemp());
            reverse(refs.begin(), refs.end());
            instructions.push_back(new AllocClosure{function, refs});
            assign(RetVal{});
            break;
          }
          case BC::Operation::Goto: {
            Label label = nextLabel();
            addLabel(label, instruction.operand0.value());
            instructions.push_back(new Jump{label});
            break;
          }
          case BC::Operation::If: {
            Label label = nextLabel();
            addLabel(label, instruction.operand0.value());
            Temp cond = popTemp();
            instructions.push_back(new CallHelper<Helper::AssertBool>{cond});
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
    }

  public:
    Compiler(shared_ptr<BC::Function> bytecode) : bytecode(bytecode) {}

    InstructionList compile() {
      compile(*bytecode);
      outputLabels();
      return labelledInstructions;
    }
  };
}
