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
  class CompilerLocalsScanner : public CompilerScanner {
  protected:
    set<string> globals;
    set<string> locals;
  public:
    CompilerLocalsScanner(set<string> globals) : globals(globals) {}
    vector<string> getLocals();
    void visit(Assignment& assign) override;
  };
}
