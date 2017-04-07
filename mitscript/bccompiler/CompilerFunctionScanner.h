#pragma once

#include "../AST.h"
#include "CompilerScanner.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class CompilerFunctionScanner : public CompilerScanner {
  protected:
    shared_ptr<FunctionLinkedList> functions;
    Function& current() {
      return *(functions->function);
    }

  public:
    CompilerFunctionScanner(shared_ptr<FunctionLinkedList> functions) : functions(functions) {};
    void visit(Name& name) override;
    void visit(Assignment& assign) override;
    void visit(Global& global) override;
    void visit(AST::Function& func) override;
  };
}
