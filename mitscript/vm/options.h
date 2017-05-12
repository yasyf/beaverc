#pragma once

#include <stddef.h>

#define OPTIMIZATION_MACHINE_CODE         (1 << 0)
#define OPTIMIZATION_STRING_TREES         (1 << 1)
#define OPTIMIZATION_COMPILE_ONLY         (1 << 2)
#define OPTIMIZATION_OPTIMIZATION_PASSES  (1 << 3)

#define OPTION_COMPILE_ERRORS       (1 << 0)
#define OPTION_SHOW_MEMORY_USAGE    (1 << 1)
#define OPTION_SHOW_MEMORY_TRACE    (1 << 2)

bool has_optimization(size_t option);
void set_optimization(size_t option);

bool has_option(size_t option);
void set_option(size_t option);