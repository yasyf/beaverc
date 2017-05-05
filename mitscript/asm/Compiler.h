#pragma once
#include "include/x64asm.h"
#include "../ir/Instructions.h"
#include "../vm/Value.h"

using namespace std;
using namespace x64asm;

namespace ASM {
  class Compiler {
    IR::InstructionList ir;
    VM::ClosureFunctionValue closure;
    x64asm::Function function;

    void compile(IR::InstructionList& ir) {
      Assembler assm;
      assm.start(function);
      assm.xor_(rcx, rcx);
      assm.finish();
    }

  public:
    Compiler(IR::InstructionList ir, VM::ClosureFunctionValue closure) : ir(ir), closure(closure) {}

    x64asm::Function compile() {
      compile(ir);
      return function;
    }
  };
}
