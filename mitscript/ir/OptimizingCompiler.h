#pragma once
#include <set>
#include "../bccompiler/Types.h"
#include "Instructions.h"
#include "Compiler.h"

#include "PropagateTypesOptimization.h"
#include "ShortJumpOptimization.h"
#include "IntAddOptimization.h"
#include "ConstantFoldingOptimization.h"
#include "RemoveObsoleteOptimization.h"
#include "RemoveNoopOptimization.h"
#include "CopyOptimization.h"

using namespace std;

namespace IR {
  const size_t NUM_PASSES = 2;

  class OptimizingCompiler {
    Compiler compiler;
    shared_ptr<BC::Function> bytecode;
    InstructionList& instructions;
    set<size_t> obsolete;

    template<typename T>
    void optimize() {
      T t;
      t.optimize(bytecode, instructions);

      for (auto temp : t.obsolete)
        obsolete.insert(temp);
    }

    void removeObsolete() {
      RemoveObsoleteOptimization opt;
      opt.obsolete = this->obsolete;
      opt.optimize(bytecode, instructions);
      this->obsolete.clear();
    }

    void runAllPasses() {
      for (size_t i = 0; i < NUM_PASSES; ++i) {
        optimize<PropagateTypesOptimization>();
        optimize<ConstantFoldingOptimization>();
        optimize<IntAddOptimization>();
        optimize<PropagateTypesOptimization>();
        removeObsolete();
        optimize<CopyOptimization>();
        optimize<RemoveNoopOptimization>();
        optimize<ShortJumpOptimization>();
      }
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
