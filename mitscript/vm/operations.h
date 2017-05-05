#pragma once

#include "Value.h"

namespace VM {
    Value add(Value left, Value right);
    Value equals(Value left, Value right);
}