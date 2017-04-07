#pragma once

#include "../AST.h"
#include "CompilerScanner.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class CompilerErrorsScanner : public CompilerScanner {
  protected:
    shared_ptr<Function> function;
  public:
    CompilerErrorsScanner(shared_ptr<Function> function) : function(function) {};
    void visit(Name& name) override;
    void visit(AST::Function& func) override;
  };
}
