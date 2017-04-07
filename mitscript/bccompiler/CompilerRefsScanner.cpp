#include "CompilerRefsScanner.h"

using namespace std;
using namespace AST;

namespace BC {
  // Search F's descendents for F's locals
  CompilerRefsScanner::CompilerRefsScanner(vector<string> locals) {
    needles = set<string>(locals.begin(), locals.end());
    ignores.push(set<string>());

    for (string name : locals)
      ignores.top().insert(name);
  }

  // Search F and F's descendents for F_0's refs
  CompilerRefsScanner::CompilerRefsScanner(vector<string> ref0s, vector<string> locals, vector<string> globals) {
    needles = set<string>(ref0s.begin(), ref0s.end());
    for (string& l : locals)
      needles.erase(l);
    for (string& g : globals)
      needles.erase(g);

    ignores.push(set<string>());
  }

  vector<string> CompilerRefsScanner::getRefs() {
    return vector<string>(refs.begin(), refs.end());
  }

  void CompilerRefsScanner::visit(Name& name) {
    if (assigning) {
      ignores.top().insert(name.name);
      return;
    }
    if (needles.count(name.name) && !ignores.top().count(name.name)) {
      refs.insert(name.name);
    }
  }

  void CompilerRefsScanner::visit(Global& global) {
    ignores.top().insert(global.name);
  }

  void CompilerRefsScanner::visit(AST::Function& func) {
    ignores.push(set<string>());
    for (Name *name : func.arguments) {
      ignores.top().insert(name->name);
    }
    scan(func.body);
    ignores.pop();
  }
}
