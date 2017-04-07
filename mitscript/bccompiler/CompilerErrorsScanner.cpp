#include <iostream>
#include <string>
#include "CompilerErrorsScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;
using namespace AST;

namespace BC {
  void CompilerErrorsScanner::visit(Name& name) {
    if (
      index(function->free_vars_, name.name) ||
      index(function->local_vars_, name.name) ||
      index(function->names_, name.name)
    ) {
      return;
    }
    throw UninitializedVariableException(name.name);
  }

  void CompilerErrorsScanner::visit(AST::Function& func) { }
}
