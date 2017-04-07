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
  class CompilerRefsScanner : public CompilerScanner {
  protected:
    set<string> needles;
    stack<set<string>> ignores;
  public:
    set<string> refs;
    CompilerRefsScanner(vector<string> locals);
    CompilerRefsScanner(vector<string> ref0s, vector<string> locals, vector<string> globals);
    vector<string> getRefs();
    void visit(Name& name) override;
    void visit(Global& global) override;
    void visit(AST::Function& func) override;
  };
}
