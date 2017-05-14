#pragma once
#include "Instructions.h"
#include "Compiler.h"
#include <set>

using namespace std;

namespace IR {
  class Optimization {
  public:
    Compiler& compiler;
    set<size_t> obsolete;

    Optimization(Compiler& compiler) : compiler(compiler) {}
    virtual void optimize() = 0;
  };
}
