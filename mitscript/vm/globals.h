#include "Interpreter.h"

extern VM::Interpreter* interpreter;

extern int run_options;

#define OPTION_MACHINE_CODE         (1 << 0)
#define OPTION_STRING_TREES         (1 << 1)
#define OPTION_COMPILE_ONLY         (1 << 2)
#define OPTION_OPTIMIZATION_PASSES  (1 << 3)
#define OPTION_NO_COMPILE_ERRORS    (1 << 4)

bool has_option(size_t option);
void set_option(size_t option);
