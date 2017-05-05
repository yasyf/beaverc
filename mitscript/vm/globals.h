#include "Interpreter.h"

extern VM::Interpreter* interpreter;

extern int run_options;

#define OPTION_MACHINE_CODE_ONLY 0x1

bool has_option(size_t option);
void set_option(size_t option);