#pragma once

#include <map>
#include <set>
#include "Value.h"

using namespace std;

class StackFrame {
  Value *ret = nullptr;
  map<string, Value*> vars;

public:
  StackFrame *parent;
  set<string> globals;

  StackFrame(StackFrame *parent) : parent(parent), vars(), globals() {}
  StackFrame() : StackFrame(nullptr) {}

  void Update(string var, Value *val) {
    vars[var] = val;
  }

  void Initialize(string var) {
    vars[var] = NoneValue::Singleton();
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

  bool returned() {
    return this->ret != nullptr;
  }

  StackFrame* CreateChild() {
    return new StackFrame(this);
  }
};
