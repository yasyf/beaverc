#pragma once

#include <map>
#include <stack>
#include "Value.h"
#include "Exception.h"

class StackFrame {
  Value *ret;
  map<string, Value*> vars;

public:
  StackFrame *parent;
  set<string> globals;

  StackFrame(StackFrame *parent) : parent(parent), vars(), globals() {}

  void Update(string var, Value *val) {
    vars[var] = val;
  }

  void Initialize(string var) {
    vars[var] = &NoneSingleton;
  }

  bool Contains(string var) {
    return vars.count(var) > 0;
  }

  Value* Read(string var) {
    if (Contains(var)) {
      return vars[var];
    } else {
      return nullptr;
    }
  }

  void SetReturn(Value *ret) {
    this->ret = ret;
  }

  Value* GetReturn() {
    return this->ret;
  }

  StackFrame* CreateChild() {
    return new StackFrame(this);
  }
};

class Heap {
  std::stack<StackFrame *> frames;

public:
  StackFrame *global;

  Heap() : global(nullptr), frames() {
    this->frames.push(this->global);
  }

  StackFrame* current() {
    return this->frames.top();
  }

  void push(StackFrame* frame) {
    frames.push(frame);
  }

  StackFrame* pop() {
    return frames.pop();
  }

  void UpdateVar(string var, Value val) {
    if (current()->globals.count(var)) {
      global.Update(var, val);
    } else {
      current()->Update(var, val);
    }
  }

  Value ReadVar(string var) {
    Value *val;
    StackFrame *curr = current();
    while (curr) {
      if (curr->globals.count(var)) {
        return global.Read(var);
      } else if (curr->Contains(var)) {
        return curr->Read(var);
      } else {
        curr = curr->parent;
      }
    }
    throw UninitializedVariableException(var);
  }
};
