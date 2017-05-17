#pragma once
#include <set>
#include "../bccompiler/Types.h"
#include "Instructions.h"
#include "Compiler.h"

#include "PropagateTypesOptimization.h"
#include "ShortJumpOptimization.h"
#include "TypeSpecializationOptimization.h"
#include "ConstantFoldingOptimization.h"
#include "RemoveObsoleteOptimization.h"
#include "RemoveNoopOptimization.h"
#include "CopyOptimization.h"
#include "LoadParamsOptimization.h"
#include "VarLivenessOptimization.h"
#include "TempLivenessOptimization.h"
#include "DeadVariableAssignmentOptimization.h"
#include "DeadTempOptimization.h"
#include "RegisterAllocationOptimization.h"

using namespace std;

namespace IR {
  const size_t NUM_PASSES = 2;

  class OptimizingCompiler {
  public:
    Compiler compiler;
    set<size_t> obsolete;

  private:
    template<typename T>
    void optimize() {
      T t(compiler);
      t.optimize();

      for (auto temp : t.obsolete)
        obsolete.insert(temp);
    }

    void removeObsolete() {
      RemoveObsoleteOptimization opt(compiler, this->obsolete);
      opt.optimize();
      this->obsolete.clear();
    }

    void runAllPasses() {
      optimize<LoadParamsOptimization>();

      for (size_t i = 0; i < NUM_PASSES; ++i) {
        optimize<PropagateTypesOptimization>();
        optimize<ConstantFoldingOptimization>();
        optimize<TypeSpecializationOptimization>();
        optimize<PropagateTypesOptimization>();
        removeObsolete();

        optimize<RemoveNoopOptimization>();

        optimize<VarLivenessOptimization>();
        optimize<DeadVariableAssignmentOptimization>();
        removeObsolete();

        optimize<TempLivenessOptimization>();
        optimize<DeadTempOptimization>();
        removeObsolete();

        optimize<RemoveNoopOptimization>();
      }

      optimize<ShortJumpOptimization>();

      optimize<VarLivenessOptimization>();
      optimize<TempLivenessOptimization>();
      optimize<RegisterAllocationOptimization>();
    }

  public:
    OptimizingCompiler(shared_ptr<BC::Function> bytecode, InstructionList& instructions)
      : compiler(bytecode, instructions)
    {}

    size_t compile(bool optimize = true) {
      compiler.compile();
      if (optimize)
        runAllPasses();
      return compiler.temps.size();
    }
  };
}
