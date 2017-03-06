#pragma once

#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include "AST.h"

using namespace std;

class Value {
  virtual string toString() = 0;
  virtual bool equals(Value *other) = 0;
};

class NoneValue : public Value {
  string toString() override {
    return "None";
  }

  bool equals(Value *other) override {
    return dynamic_cast<NoneValue>(other) != nullptr;
  }
};
const NoneSingleton = NoneValue();

template <typename T>
class ConstantValue : public Value {
public:
  T value;

  ConstantValue(T value) : value(value) {}

  bool equals(Value *other) override {
    T *otherT = dynamic_cast<T*>(other);
    return otherT != nullptr && otherT->value == value;
  }
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
  vector<string> arguments;

  FunctionValue(StackFrame *frame, Block *code, vector<string> arguments) : frame(frame), code(code), arguments(arguments) {}

  string toString() override {
    return "FUNCTION";
  }

  bool equals(Value *other) override {
    FunctionValue *func = dynamic_cast<FunctionValue*>(other);
    return func != nullptr && func->frame == frame && func->code == code;
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

  bool equals(Value *other) override {
    return this == dynamic_cast<RecordValue*>(other);
  }

  void Update(string key, Value *val) {
    record[key] = val;
  }

  Value* Read(string key) {
    return record.count() ? record[key] : &NoneSingleton;
  }
}

