#pragma once

#include <map>
#include <stack>
#include "Value.h"
#include "Stack.h"
#include "Exception.h"

namespace BasicInterpreter {
  class Heap {
    std::stack<StackFrame *> frames;

  public:
    StackFrame *global;

    Heap() : global(new StackFrame()), frames() {
      this->frames.push(this->global);
    }

    StackFrame* current() {
      return this->frames.top();
    }

    void push(StackFrame* frame) {
      frames.push(frame);
    }

    StackFrame* pop() {
      StackFrame *frame = frames.top();
      frames.pop();
      return frame;
    }

    void UpdateVar(string var, Value *val) {
      if (current()->globals.count(var)) {
        global->Update(var, val);
      } else {
        current()->Update(var, val);
      }
    }

    Value* ReadVar(string var) {
      StackFrame *curr = current();
      while (curr) {
        if (curr->globals.count(var)) {
          return global->Read(var);
        } else if (curr->Contains(var)) {
          return curr->Read(var);
        } else {
          curr = curr->parent;
        }
      }
      throw UninitializedVariableException(var);
    }
  };
}
