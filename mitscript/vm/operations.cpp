#include "operations.h"
#include "globals.h"

namespace VM {
    Value add(Value left, Value right) {
        if (right.isString() || left.isString()) {
            return Value::makeString(interpreter->heap.allocate<StringValue>(left.toString() + right.toString()));
        } else if (right.isInteger() && left.isInteger()) {
            return Value::makeInteger(right.getInteger() + left.getInteger());
        } else {
            throw IllegalCastException("Can't perform addition");
        }
    }

    Value equals(Value left, Value right) {
        return Value::makeBoolean(left == right);
    }
}