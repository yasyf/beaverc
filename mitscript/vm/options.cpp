#include "options.h"

static int options = 0;
static int optimizations = 0;

bool has_optimization(size_t optimization) {
    return (optimizations & optimization);
}

void set_optimization(size_t optimization) {
    optimizations |= optimization;
}

bool has_option(size_t option) {
    return (options & option);
}

void set_option(size_t option) {
    options |= option;
}
