#pragma once
#include <iostream>

using namespace std;

namespace ASM {
  class PrettyPrinter {
    x64asm::Function function;

  public:
    PrettyPrinter(x64asm::Function function) : function(function) {}

    void print() {
      cout << function << endl;
    }
  };
}
