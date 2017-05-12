#include "operations.h"
#include "globals.h"

namespace VM {
    Value add(Value left, Value right) {
        if (right.isString() || left.isString()) {
            if (has_optimization(OPTIMIZATION_STRING_TREES)) {
                return Value::makeString(interpreter->heap.allocate<StringValue>(left, right));
            } else {
                return Value::makeString(interpreter->heap.allocate<StringValue>(left.toString() + right.toString()));
            }
        } else if (right.isInteger() && left.isInteger()) {
            return Value::makeInteger(right.getInteger() + left.getInteger());
        } else {
            throw IllegalCastException("Can't perform addition");
        }
    }

    Value equals(Value left, Value right) {
        return Value::makeBoolean(left == right);
    }

    Value allocateFunction(ClosureFunctionValue* closure, int index) {
      if (index < 0 && (-index - 1) < static_cast<int>(BuiltInFunctionType::MAX)) {
        return Value::makePointer(interpreter->heap.allocate<BuiltInFunctionValue>(-index - 1));
      } else {
        return Value::makePointer(interpreter->heap.allocate<BareFunctionValue>(safe_index(closure->value->functions_, index)));
      }
    }
}
