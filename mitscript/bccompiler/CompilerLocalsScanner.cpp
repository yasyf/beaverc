#include "CompilerLocalsScanner.h"

using namespace std;
using namespace AST;

namespace BC {
  vector<string> CompilerLocalsScanner::getLocals() {
    return vector<string>(locals.begin(), locals.end());
  }

  void CompilerLocalsScanner::visit(Assignment& assign) {
    if (Name *name = dynamic_cast<Name*>(assign.lhs)) {
      if (!globals.count(name->name))
        locals.insert(name->name);
    }
  }
}
