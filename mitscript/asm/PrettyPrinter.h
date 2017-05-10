#pragma once
#include <iostream>
#include "extern.h"

using namespace std;

namespace ASM {
  class PrettyPrinter {
    x64asm::Function function;

  public:
    PrettyPrinter(x64asm::Function function) : function(function) {}

    void print() {
      ud_t u;
      ud_init(&u);
      ud_set_input_buffer(&u, (const uint8_t*) function.data(), function.size());
      ud_set_mode(&u, 64);
      ud_set_syntax(&u, UD_SYN_INTEL);
      while (ud_disassemble(&u)) {
        cout << ud_insn_asm(&u) << endl;
      }
    }
  };
}
