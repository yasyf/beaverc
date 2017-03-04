#pragma once

#include <string>
#include <sstream>
#include <map>
#include <set>
#include "AST.h"

using namespace std;

class Value {
  virtual string toString() = 0;
};

class NoneValue : public Value {
  string toString() override {
    return "None";
  }
};
const NoneSingleton = NoneValue();

template <typename T>
class ConstantValue : public Value {
public:
  T value;

  ConstantValue(T value) : value(value) {}
};

class BooleanValue : public ConstantValue<bool> {
  string toString() override {
    return value ? "true" : "false";
  }
};

class IntegerValue : public ConstantValue<int> {
  string toString() override {
    return to_string(value);
  }
};

class StringValue : public ConstantValue<string> {
  string toString() override {
    return value;
  }
};

class FunctionValue : public Value {
public:
  StackFrame *frame;
  Block *code;

  FunctionValue(StackFrame *frame, Block *code) : frame(frame), code(code) {}

  string toString() override {
    return "FUNCTION";
  }
}

class RecordValue : public Value {
public:
  map<string, Value*> record;

  RecordValue() : record(record) {}

  string toString() override {
    ostringstream oss;
    oss << "{";
    for (auto& kv : record) {
      oss << kv.first << ":" << kv.second.toString() << " ";
    }
    oss << "}";
    return oss.str();
  }

  void Update(string key, Value *val) {
    record[key] = val;
  }

  Value* Read(string key) {
    return record.count() ? record[key] : &NoneSingleton;
  }
}

