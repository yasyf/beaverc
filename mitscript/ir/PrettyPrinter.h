#pragma once
#include <iostream>
#include "Instructions.h"

using namespace std;

namespace IR {
  class PrettyPrinter {
    InstructionList instructions;

  public:
    PrettyPrinter(InstructionList instructions) : instructions(instructions) {}

    void print() {
      for (auto instruction : instructions) {
        cout << instruction->toString() << endl;
      }
    }
  };
}
