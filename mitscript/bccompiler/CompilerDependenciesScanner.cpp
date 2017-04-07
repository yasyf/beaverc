#include <iostream>
#include <string>
#include "CompilerDependenciesScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;
using namespace AST;

namespace BC {
  void CompilerDependenciesScanner::visit(Name& name) {
    if (
      index(function->free_vars_, name.name) ||
      index(function->local_vars_, name.name) ||
      index(function->names_, name.name)
    ) {
      return;
    }

    if (root->storing == name.name) {
      // Special case for recursive define at top level
      insert(function->names_, name.name);
    } else if (index(root->function->names_, name.name)) {
      // Since root vars are globals, they don't get passed as refs
      insert(function->names_, name.name);
    } else {
      throw UninitializedVariableException(name.name);
    }
  }

  void CompilerDependenciesScanner::visit(AST::Function& func) { }
}
