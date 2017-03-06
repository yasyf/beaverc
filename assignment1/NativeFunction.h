#pragma once

#include <vector>
#include <functional>
#include <iostream>
#include "Value.h"

using namespace std;

class NativeFunction : public FunctionValue {
  static StackFrame *sharedFrame = new StackFrame();
  static Block *sharedBlock = new Block();
  function<Value*(vector<Value *>)> &func;

public:

  NativeFunction(vector<string> arguments, F &func) : FunctionValue(sharedFrame, sharedBlock, arguments), func(func) {}

  Value* call(vector<Value *> args) {
    return func(args);
  }
};
