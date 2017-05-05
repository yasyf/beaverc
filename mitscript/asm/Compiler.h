#pragma once
#include "include/x64asm.h"
#include "../ir/Instructions.h"
#include "../vm/Value.h"

using namespace std;
using namespace x64asm;
using namespace IR;

namespace ASM {
  class Compiler {
    IR::InstructionList& ir;
    VM::ClosureFunctionValue& closure;
    x64asm::Function function;
    Assembler assm;
    size_t num_locals;

    M64 local(size_t i) {
      return M64{rbp, Imm32{(uint32_t)(-(i+1) * 8)}};
    }

    M64 temp(size_t i) {
      return M64{rbp, Imm32{(uint32_t)(-(i+1+num_locals) * 8)}};
    }

    void read_local(size_t i, const R64& reg = r10) {
      assm.mov(local(i), reg);
    }

    void read_temp(size_t i, const R64& reg = r10) {
      assm.mov(temp(i), reg);
    }

    void write_local(size_t i, const R64& reg = r10) {
      assm.mov(reg, local(i));
    }

    void write_temp(size_t i, const R64& reg = r10) {
      assm.mov(reg, temp(i));
    }

    void compile(IR::InstructionList& ir) {
      assm.start(function);
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              read_local(assign->src.num);
              write_temp(assign->dest.num);
            }
            break;
          }
        }
      }
      assm.finish();
    }

  public:
    Compiler(IR::InstructionList& ir, VM::ClosureFunctionValue& closure) : ir(ir), closure(closure) {
      this->num_locals = closure.value->local_vars_.size();
    }

    x64asm::Function compile() {
      compile(ir);
      return function;
    }
  };
}
