#pragma once

#include <map>
#include <stack>
#include "Value.h"

class Heap {
  std::stack<StackFrame *> frames;

public:
  StackFrame *global;

  Heap() : global(), frames() {
    this->frames.push(this->global);
  }

  StackFrame* current() {
    return this->frames.top();
  }

  void UpdateVar(string var, Value val) {
    if (current().globals.count(var)) {
      global.Update(var, val);
    } else {
      current().Update(var, val);
    }
  }

  Value ReadVar(string var) {
    if (current().globals.count(var)) {
      return global.Read(var);
    } else {
      return current().Read(var);
    }
  }
};
