#pragma once

#include <map>
#include <stack>
#include "Value.h"
#include "Exception.h"

class StackFrame {
  StackFrame *parent;
  map<string, Value> vars;
  set<string> globals;

public:
  StackFrame() : parent(nullptr), vars(), globals() {
    // TODO: add print, input, and intcast globals
  }

  void Update(string var, Value val) {
    vars[var] = val;
  }

  Value Read(string var) {
    if (vars.count(var)) {
      return vars[var];
    } else {
      if (parent) {
        return parent.Read(var);
      } else {
        throw UninitializedVariableException(var);
      }
    }
  }
};

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
