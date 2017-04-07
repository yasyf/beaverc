#pragma once

#include <string>
#include <set>
#include <stack>
#include "../AST.h"
#include "CompilerScanner.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class CompilerLocalRefScanner : public CompilerScanner {
  protected:
    set<string> local_vars;
    stack<set<string>> nonlocals;

  public:
    set<string> local_var_refs;
    CompilerLocalRefScanner(vector<string> vars);
    void visit(Name& name) override;
    void visit(Global& global) override;
    void visit(AST::Function& func) override;
  };
}
