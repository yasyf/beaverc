#pragma once

#include <string>
#include <map>
#include <set>
#include "AST.h"
#include "Exception.h"

using namespace std;

class Value {};

class NoneValue : public Value {};

template <typename T>
class ConstantValue : public Value {
public:
  T value;

  ConstantValue(T value) : value(value) {}
};

using BooleanValue = ConstantValue<bool>;
using IntegerValue = ConstantValue<int>;
using StringValue = ConstantValue<string>;

class StackFrame : public Value {
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

class FunctionValue : public Value {
public:
  StackFrame *frame;
  Block *code;

  FunctionValue(StackFrame *frame, Block *code) : frame(frame), code(code) {}
}

class RecordValue : public Value {
public:
  map<string, Value*> record;

  RecordValue() : record(record) {}
}

