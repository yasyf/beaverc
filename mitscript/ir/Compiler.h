#pragma once
#include "../bccompiler/Instructions.h"
#include "Instructions.h"
#include <algorithm>
#include <vector>
#include <stack>

using namespace std;

namespace IR {
  class Compiler {
    stack<Temp> operands;
    shared_ptr<BC::Function> bytecode;
    InstructionList instructions;
    size_t temp_count = 0;

    Temp popTemp() {
      Temp t = operands.top();
      operands.pop();
      return t;
    }

    Temp nextTemp() {
      Temp t = Temp{temp_count++};
      operands.push(t);
      return t;
    }

    template<typename T>
    void assign(T t) {
      instructions.push_back(new Assign<T>{nextTemp(), t});
    }

    template<typename T>
    void store(T t) {
      instructions.push_back(new Store<T>{t, popTemp()});
    }

    void compile(BC::Function& func) {
      for (size_t i = func.parameter_count_; i < func.local_vars_.size(); ++i) {
        instructions.push_back(new AllocVar{Var{i}, sizeof(int64_t)});
      }

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
            assign(Glob{func.names_[instruction.operand0.value()]});
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
            store(Glob{func.names_[instruction.operand0.value()]});
            break;
          case BC::Operation::Add: {
            // TODO: handle non-ints
            Temp arg1 = popTemp();
            Temp arg2 = popTemp();
            instructions.push_back(new Add{nextTemp(), arg1, arg2});
            break;
          }
          case BC::Operation::Return:
            instructions.push_back(new Return{popTemp()});
            break;
        }
      }
    }

  public:
    Compiler(shared_ptr<BC::Function> bytecode) : bytecode(bytecode) {}

    InstructionList compile() {
      compile(*bytecode);
      return instructions;
    }
  };
}
