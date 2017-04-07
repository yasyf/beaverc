#pragma once

#include "../AST.h"
#include "CompilerScanner.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class CompilerDependenciesScanner : public CompilerScanner {
  protected:
    shared_ptr<FunctionLinkedList> functions;
  public:
    CompilerDependenciesScanner(shared_ptr<FunctionLinkedList> functions) : functions(functions) {};
    shared_ptr<Function> current() {
      return functions->function;
    }
    void visit(Name& name) override;
    void visit(AST::Function& func) override;
  };
}
