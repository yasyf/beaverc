#pragma once
#include "../bccompiler/Types.h"
#include "Instructions.h"
#include "Compiler.h"

#include "ShortJumpOptimization.h"

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
      optimize<ShortJumpOptimization>();
    }

  public:
    OptimizingCompiler(shared_ptr<BC::Function> bytecode, InstructionList& instructions)
      : compiler(bytecode, instructions), bytecode(bytecode), instructions(instructions)
    {}

    size_t compile() {
      size_t num_temps = compiler.compile();
      runAllPasses();
      return num_temps;
    }
  };
}
