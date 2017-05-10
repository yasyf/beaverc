#pragma once
#include "../bccompiler/Types.h"
#include "Instructions.h"
#include "Compiler.h"

#include "ShortJumpOptimization.h"
#include "IntAddOptimization.h"

using namespace std;

namespace IR {
  class OptimizingCompiler {
    Compiler compiler;
    shared_ptr<BC::Function> bytecode;
    InstructionList& instructions;

    template<typename T>
    void optimize() {
      T t;
      t.optimize(bytecode, instructions);
    }

    void runAllPasses() {
      optimize<IntAddOptimization>();
      optimize<ShortJumpOptimization>();
    }

  public:
    OptimizingCompiler(shared_ptr<BC::Function> bytecode, InstructionList& instructions)
      : compiler(bytecode, instructions), bytecode(bytecode), instructions(instructions)
    {}

    size_t compile(bool optimize = true) {
      size_t num_temps = compiler.compile();
      if (optimize)
        runAllPasses();
      return num_temps;
    }
  };
}
