#pragma once

#include "Value.h"

namespace VM {
    template<typename T>
    static T safe_index(const std::vector<T> &v, int i) {
        if (i >= 0 && i < v.size()) {
            return v[i];
        }
        throw RuntimeException("Tried to access an index out of bounds.");
    }

    Value allocateFunction(ClosureFunctionValue* closure, int index);
    Value add(Value left, Value right);
    Value equals(Value left, Value right);
}