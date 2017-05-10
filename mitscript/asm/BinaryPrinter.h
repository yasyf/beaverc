#pragma once
#include <iostream>
#include "extern.h"

using namespace std;

namespace ASM {
  class BinaryPrinter {
    x64asm::Function function;

  public:
    BinaryPrinter(x64asm::Function function) : function(function) {}

    void print() {
      cout << function << endl;
    }
  };
}
