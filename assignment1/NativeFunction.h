#pragma once

#include <vector>
#include <functional>
#include <iostream>
#include "Value.h"
#include "Stack.h"

using namespace std;

class NativeFunction : public FunctionValue {
  function<Value*(vector<Value*>)> func;

public:

  NativeFunction(vector<string> arguments, function<Value*(vector<Value*>)> func)
    : FunctionValue(new StackFrame(), new Block(), arguments), func(func) {}

  Value* call(vector<Value *> args) {
    return func(args);
  }
};