#pragma once
#include "../bccompiler/Instructions.h"
#include "Instructions.h"

namespace IR {
  class Compiler {
    shared_ptr<BC::Function> bytecode;
    InstructionList instructions;
    size_t reg_count = 0;

    Reg lastReg(size_t count = 1) {
      return Reg{reg_count - count};
    }

    Reg nextReg() {
      return Reg{reg_count++};
    }

    void compile(BC::Function& func) {
      for (auto it = func.local_vars_.begin() + func.parameter_count_; it != func.local_vars_.end(); ++it) {
        instructions.push_back(new AllocVar{Var{*it}, sizeof(int64_t)});
      }

      for (auto instruction : func.instructions) {
        switch(instruction.operation) {
          case BC::Operation::LoadFunc:
            // TODO: compile and store func, get pointer
            instructions.push_back(new StoreReg<Const>{nextReg(), Const{0}});
            break;
          case BC::Operation::LoadGlobal:
            instructions.push_back(new StoreReg<Var>{nextReg(), Var{func.names_[instruction.operand0.value()]}});
            break;
          case BC::Operation::LoadLocal:
            instructions.push_back(new StoreReg<Var>{nextReg(), Var{func.local_vars_[instruction.operand0.value()]}});
            break;
          case BC::Operation::LoadConst:
            instructions.push_back(new StoreReg<Const>{nextReg(), Const{func.constants_[instruction.operand0.value()]}});
            break;
          case BC::Operation::Add:
            // this will probably break on more complex stuff that uses intermediary regs
            instructions.push_back(new Add{nextReg(), lastReg(2), lastReg()});
            break;
          case BC::Operation::StoreLocal:
            instructions.push_back(new StoreVar<Reg>{Var{func.local_vars_[instruction.operand0.value()]}, lastReg()});
            break;
          case BC::Operation::StoreGlobal:
            instructions.push_back(new StoreVar<Reg>{Var{func.names_[instruction.operand0.value()]}, lastReg()});
            break;
          case BC::Operation::Return:
            instructions.push_back(new Return{lastReg()});
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
