#include "CompilerLocalRefScanner.h"

using namespace std;
using namespace AST;

namespace BC {
  CompilerLocalRefScanner::CompilerLocalRefScanner(vector<string> vars) {
    this->local_vars = set<string>(vars.begin(), vars.end());
    this->nonlocals.push(set<string>());

    for (string v : vars)
      nonlocals.top().insert(v);
  }

  void CompilerLocalRefScanner::visit(Name& name) {
    if (local_vars.count(name.name) && !nonlocals.top().count(name.name)) {
      local_var_refs.insert(name.name);
    }
  }

  void CompilerLocalRefScanner::visit(Global& global) {
    nonlocals.top().insert(global.name);
  }

  void CompilerLocalRefScanner::visit(AST::Function& func) {
    nonlocals.push(set<string>());
    for (Name *name : func.arguments) {
      nonlocals.top().insert(name->name);
    }
    scan(func.body);
    nonlocals.pop();
  }
}
