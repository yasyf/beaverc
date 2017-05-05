#include "globals.h"

VM::Interpreter* interpreter;

int run_options = 0;

bool has_option(size_t option) {
    return (run_options & option);
}

void set_option(size_t option) {
    run_options |= option;
}