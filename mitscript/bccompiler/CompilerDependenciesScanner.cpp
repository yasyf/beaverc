#include <iostream>
#include <string>
#include "CompilerDependenciesScanner.h"
#include "Util.h"

using namespace std;
using namespace AST;

namespace BC {
  void CompilerDependenciesScanner::visit(Name& name) {
    if (
      index(current()->free_vars_, name.name) ||
      index(current()->local_vars_, name.name) ||
      index(current()->names_, name.name)
    ) {
      return;
    }

    // Globals don't get passed as refs, so we traverse up the tree
    shared_ptr<FunctionLinkedList> node = functions->last;
    shared_ptr<FunctionLinkedList> root;

    while (node) {
      if (index(node->function->names_, name.name)) {
        insert(current()->names_, name.name);
        return;
      }
      root = node;
      node = node->last;
    }

    // Special case for recursive define at top level
    if (root->storing == name.name) {
      insert(current()->names_, name.name);
    }
  }

  void CompilerDependenciesScanner::visit(AST::Function& func) { }
}
