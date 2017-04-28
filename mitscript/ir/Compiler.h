#pragma once
#include "../bccompiler/Instructions.h"
#include "Instructions.h"
#include <stack>

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

    void compile(BC::Function& func) {
      for (auto it = func.local_vars_.begin() + func.parameter_count_; it != func.local_vars_.end(); ++it) {
        instructions.push_back(new AllocVar{Var{*it}, sizeof(int64_t)});
      }

      for (auto instruction : func.instructions) {
        switch(instruction.operation) {
          case BC::Operation::LoadFunc:
            // TODO: compile and store func, get pointer
            instructions.push_back(new AssignTemp<Const>{nextTemp(), Const{0}});
            break;
          case BC::Operation::LoadGlobal:
            instructions.push_back(new AssignTemp<Glob>{nextTemp(), Glob{func.names_[instruction.operand0.value()]}});
            break;
          case BC::Operation::LoadLocal:
            instructions.push_back(new AssignTemp<Var>{nextTemp(), Var{func.local_vars_[instruction.operand0.value()]}});
            break;
          case BC::Operation::LoadConst:
            instructions.push_back(new AssignTemp<Const>{nextTemp(), Const{func.constants_[instruction.operand0.value()]}});
            break;
          case BC::Operation::Add: {
            // TODO: handle non-ints
            Temp arg1 = popTemp();
            Temp arg2 = popTemp();
            instructions.push_back(new Add{nextTemp(), arg1, arg2});
            break;
          }
          case BC::Operation::StoreLocal:
            instructions.push_back(new StoreVar<Temp>{Var{func.local_vars_[instruction.operand0.value()]}, popTemp()});
            break;
          case BC::Operation::StoreGlobal:
            instructions.push_back(new StoreGlob<Temp>{Glob{func.names_[instruction.operand0.value()]}, popTemp()});
            break;
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
