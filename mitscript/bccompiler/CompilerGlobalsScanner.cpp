#include "CompilerGlobalsScanner.h"

using namespace std;
using namespace AST;

namespace BC {
  vector<string> CompilerGlobalsScanner::getGlobals() {
    return vector<string>(globals.begin(), globals.end());
  }

  void CompilerGlobalsScanner::visit(Global& global) {
    globals.insert(global.name);
  }
}
