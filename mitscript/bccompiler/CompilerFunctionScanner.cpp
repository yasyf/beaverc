#include <iostream>
#include <string>
#include "CompilerFunctionScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;
using namespace AST;

namespace BC {
  void CompilerFunctionScanner::visit(Name& name) {
    if (
      index(current().free_vars_, name.name) ||
      index(current().local_vars_, name.name) ||
      index(current().names_, name.name)
    ) {
      return;
    }

    shared_ptr<FunctionLinkedList> node = functions->last;
    shared_ptr<FunctionLinkedList> last_node;
    vector<shared_ptr<FunctionLinkedList>> ancestors;

    while (node) {
      if (index(node->function->local_vars_, name.name)) {
        insert(current().free_vars_, name.name);
        insert(node->local_reference_vars_, name.name);
        for (auto an : ancestors)
          insert(an->free_reference_vars_, name.name);
        return;
      } else if (index(node->function->free_vars_, name.name)) {
        insert(current().free_vars_, name.name);
        insert(node->free_reference_vars_, name.name);
        for (auto an : ancestors)
          insert(an->free_reference_vars_, name.name);
        return;
      } else if (index(node->function->names_, name.name)) {
        insert(current().names_, name.name);
        return;
      }

      ancestors.push_back(node);
      last_node = node;
      node = node->last;
    }

    // Special case for recursive define at top level
    if (last_node->storing == name.name)
      insert(current().names_, name.name);
    else
      throw UninitializedVariableException(name.name);
  }

  void CompilerFunctionScanner::visit(Assignment& assign) {
    if (Name *name = dynamic_cast<Name*>(assign.lhs)) {
      insert(current().local_vars_, name->name);
    } else {
      scan(assign.lhs);
    }
    scan(assign.expr);
  }

  void CompilerFunctionScanner::visit(Global& global) {
    insert(current().names_, global.name);
  }

  void CompilerFunctionScanner::visit(AST::Function& func) { }
}
