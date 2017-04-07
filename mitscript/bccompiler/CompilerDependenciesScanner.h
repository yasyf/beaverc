#pragma once

#include "../AST.h"
#include "CompilerScanner.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class CompilerDependenciesScanner : public CompilerScanner {
  protected:
    shared_ptr<Function> function;
    shared_ptr<FunctionLinkedList> root;
  public:
    CompilerDependenciesScanner(shared_ptr<Function> function, shared_ptr<FunctionLinkedList> root) : function(function), root(root) {};
    void visit(Name& name) override;
    void visit(AST::Function& func) override;
  };
}
