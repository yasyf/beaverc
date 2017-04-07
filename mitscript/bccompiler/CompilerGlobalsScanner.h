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
  class CompilerGlobalsScanner : public CompilerScanner {
  public:
    set<string> globals;
    vector<string> getGlobals();
    void visit(Global& global) override;
  };
}
