#pragma once

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include "../bccompiler/Types.h"
#include "../gc/CollectedHeap.h"
#include "../gc/Collectable.h"
#include "InterpreterException.h"
#include "Value.fwd.h"
#include "include/x64asm.h"

#define _INTEGER_TAG 0x0
#define _NONE_TAG 0x1
#define _BOOLEAN_TAG 0x2

#define _STRING_TAG 0x3
#define _POINTER_TAG 0x4

#define _STRING_CONSTANT_TAG _STRING_TAG
#define _STRING_VALUE_TAG (_POINTER_TAG | _STRING_TAG)

namespace VM {
  struct Interpreter;

  #define unlikely(x)     __builtin_expect((x),0)

  #define _VALUE_MASK 0x7
  #define _STRING_MASK 0x3

  #define __IS_INTEGER_VALUE(value) ((value & _VALUE_MASK) == _INTEGER_TAG)
  #define __IS_NONE_VALUE(value) ((value & _VALUE_MASK) == _NONE_TAG)
  #define __IS_BOOLEAN_VALUE(value) ((value & _VALUE_MASK) == _BOOLEAN_TAG)
  #define __IS_STRING_CONSTANT_VALUE(value) ((value & _VALUE_MASK) == _STRING_CONSTANT_TAG)
  #define __IS_STRING_VALUE(value) ((value & _VALUE_MASK) == _STRING_VALUE_TAG)

  #define __IS_POINTER_VALUE(value) ((value & _POINTER_TAG) == _POINTER_TAG)
  #define __IS_STRING(value) ((value & _STRING_TAG) == _STRING_TAG)

  struct PointerValue : GC::Collectable {
    PointerValue(GC::CollectedHeap& heap) : GC::Collectable(heap) {};
    virtual std::string toString() = 0;
  };

  struct Value {
    uint64_t value;

    Value() : value(_NONE_TAG) {};

    Value(uint64_t value) : value(value) {};


    bool isInteger() const {
      return __IS_INTEGER_VALUE(value);
    }

    bool isPointer() const {
      return __IS_POINTER_VALUE(value);
    }

    bool isStringValue() const {
      return __IS_STRING_VALUE(value);
    }

    bool isString() const {
      return __IS_STRING(value);
    }

    bool getBoolean() const {
      if (unlikely(!__IS_BOOLEAN_VALUE(value))) {
        throw IllegalCastException("Value is not a boolean");
      }
      return static_cast<bool>(value & ~_VALUE_MASK);
    };

    int64_t getInteger() const {
      if (unlikely(!__IS_INTEGER_VALUE(value))) {
        throw IllegalCastException("Value is not a integer");
      }
      return static_cast<int64_t>(value & ~_VALUE_MASK) / 8;
    };

    const char* getStringConstant() const {
      if (unlikely(!__IS_STRING_CONSTANT_VALUE(value))) {
        throw IllegalCastException("Value is not a string constant");
      }
      return reinterpret_cast<const char*>(value & ~_VALUE_MASK);
    }

    PointerValue* getPointerValue() const {
      if (unlikely(!__IS_POINTER_VALUE(value))) {
        throw IllegalCastException("Can't cast this value to a pointer type.");
      }
      return reinterpret_cast<PointerValue*>(value & ~_VALUE_MASK);
    };

    template<typename T>
    T* getPointer() const {
      if (T* t = dynamic_cast<T*>(getPointerValue())) {
        return t;
      }
      throw IllegalCastException("Can't cast the pointer to the needed type");
    }

    std::string toString() const {
      switch (value & _VALUE_MASK) {
        case _INTEGER_TAG: {
          return std::to_string(getInteger());
        }
        case _NONE_TAG: {
          return "None";
        }
        case _BOOLEAN_TAG: {
          return (getBoolean()) ? "True" : "False";
        }
        case _STRING_CONSTANT_TAG: {
          return std::string(getStringConstant());
        }
        case _STRING_VALUE_TAG:
        case _POINTER_TAG: {
          return getPointerValue()->toString();
        }
      }
    };

    bool operator==(Value other) {
      if (__IS_STRING(value) && __IS_STRING(other.value)) {
        return toString() == other.toString(); // TODO: Make this faster
      }
      return value == other.value;
    }

    static Value makeNone() {
      return Value(_NONE_TAG);
    }

    static Value makeBoolean(bool value) {
      return Value(_BOOLEAN_TAG | (static_cast<uint64_t>(value) << 3));
    }

    static Value makeInteger(int64_t value) {
      return Value(static_cast<uint64_t>(value * 8));
    }

    static Value makePointer(PointerValue* value) {
      return Value(reinterpret_cast<uint64_t>(value) | _POINTER_TAG);
    }

    static Value makeString(StringValue* value) {
      return Value(reinterpret_cast<uint64_t>(value) | _STRING_VALUE_TAG);
    }

    static Value makeStringConstant(const char* value) {
      return Value(reinterpret_cast<uint64_t>(value) | _STRING_CONSTANT_TAG);
    }
  };

  struct StringValue : public PointerValue {
    size_t height;
    char* memory;
    size_t length;
    Value left;
    Value right;

    StringValue(GC::CollectedHeap& heap, const std::string& value) : PointerValue(heap) {
      height = 0;
      length = value.size();
      memory = static_cast<char*>(malloc(length * sizeof(char)));
      strncpy(memory, value.c_str(), length);
      heap.increaseSize(size());
    }

    StringValue(GC::CollectedHeap& heap, const Value l, const Value r) : PointerValue(heap) {
      if (l.isPointer() && !l.isStringValue()) {
        // Must be a record or function
        left = Value::makeString(heap.allocate<StringValue>(l.toString()));
      } else {
        left = l;
      }
      if (r.isPointer() && !r.isStringValue()) {
        // Must be a record or function
        right = Value::makeString(heap.allocate<StringValue>(r.toString()));
      } else {
        right = r;
      }
      height = 1;
      if (left.isPointer()) {
        // Must be a string now
        StringValue* ll = left.getPointer<StringValue>();
        height = max(height, ll->height + 1);
      }
      if (right.isPointer()) {
        StringValue* rr = right.getPointer<StringValue>();
        height = max(height, rr->height + 1);
      }
    }

    ~StringValue() {
      #ifdef DEBUG
      cout << "DELETING StringValue: " << toString() << endl;
      #endif
      if (height == 0) {
        free(memory);
      }
      heap.decreaseSize(size());
    }

    std::string toString() {
      if (height == 0) {
        return std::string(memory, length);
      }
      return left.toString() + right.toString();
    };

    virtual size_t _size() {
      if (height == 0) {
        return sizeof(StringValue) + length * sizeof(char);
      } else {
        return sizeof(StringValue);
      }
    }

    virtual void markChildren() {
      if (left.isPointer()) {
        left.getPointerValue()->mark();
      }
      if (right.isPointer()) {
        right.getPointerValue()->mark();
      }
    }
  };

  struct RecordValue : public PointerValue {
    std::unordered_map<std::string, Value> values;

    RecordValue(GC::CollectedHeap& heap) : PointerValue(heap) {
      heap.increaseSize(size());
    }

    ~RecordValue() {
      #ifdef DEBUG
      cout << "DELETING RecordValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    Value get(std::string key) {
      return values[key];
    }

    void insert(std::string key, Value inserted) {
      if (values.count(key) == 0)
        heap.increaseSize(sizeof(std::string) + key.capacity() * sizeof(char) + sizeof(Value));
      values[key] = inserted;
    }

    std::string toString() {
      std::string result = "{";
      for (auto keyvalue : values) {
          result += keyvalue.first + ":" + keyvalue.second.toString() + " ";
      };
      result += "}";
      return result;
    }

    virtual size_t _size() {
      size_t s = sizeof(RecordValue);
      for (auto pair : values) {
        s += sizeof(std::string) + pair.first.capacity() * sizeof(char) + sizeof(Value);
      }
      return s;
    }

    virtual void markChildren() {
      for (auto pair : values) {
        if (pair.second.isPointer()) {
          pair.second.getPointerValue()->mark();
        }
      }
    }
  };

  struct ReferenceValue : public PointerValue {
    Value value;

    ReferenceValue(GC::CollectedHeap& heap, Value v) : PointerValue(heap), value(v) {
      heap.increaseSize(size());
    }

    ~ReferenceValue() {
      #ifdef DEBUG
      cout << "DELETING ReferenceValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    std::string toString() {
      #if DEBUG
        return "ref";
      #else
        throw RuntimeException("You have uncovered a bug :(");
      #endif
    }

    virtual size_t _size() {
      return sizeof(ReferenceValue);
    }

    virtual void markChildren() {
      if (value.isPointer()) {
        value.getPointerValue()->mark();
      }
    }
  };

  struct AbstractFunctionValue : public PointerValue {
    AbstractFunctionValue(GC::CollectedHeap& heap) : PointerValue(heap) {}

    std::string toString() { return "FUNCTION"; };
    virtual Value call(std::vector<Value> & arguments) = 0;
  };

  struct BareFunctionValue : public AbstractFunctionValue {
    std::shared_ptr<BC::Function> value;

    BareFunctionValue(GC::CollectedHeap& heap, std::shared_ptr<BC::Function> value) : AbstractFunctionValue(heap), value(value) {
      heap.increaseSize(size());
    }

    ~BareFunctionValue() {
      #ifdef DEBUG
      cout << "DELETING BareFunctionValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    Value call(std::vector<Value> & arguments);

    virtual size_t _size() {
      return sizeof(BareFunctionValue);
    }

    virtual void markChildren() {}
  };

  struct ClosureFunctionValue : public AbstractFunctionValue {
    bool is_compiled;
    std::shared_ptr<BC::Function> value;
    std::vector<ReferenceValue*> references;
    x64asm::Function compiled_func;

    ClosureFunctionValue(GC::CollectedHeap& heap, std::shared_ptr<BC::Function> value) : AbstractFunctionValue(heap), value(value) {
      is_compiled = false;
      heap.increaseSize(size());
    }

    ~ClosureFunctionValue() {
      #ifdef DEBUG
      cout << "DELETING ClosureFunctionValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }


    void add_reference(ReferenceValue* reference) {
      heap.increaseSize(sizeof(ReferenceValue*));
      references.push_back(reference);
    };

    Value call(std::vector<Value> & arguments);
    virtual size_t _size() {
      return sizeof(ClosureFunctionValue) + references.size() * sizeof(ReferenceValue*);
    }

    virtual void markChildren() {
      for (auto ref : references)
        ref->mark();
    }
  };

  enum class BuiltInFunctionType {
    Print = 0,
    Input = 1,
    Intcast = 2,
    MAX
  };

  struct BuiltInFunctionValue : public AbstractFunctionValue {
    BuiltInFunctionType type;

    BuiltInFunctionValue(GC::CollectedHeap& heap, BuiltInFunctionType type) : AbstractFunctionValue(heap), type(type) {
      heap.increaseSize(size());
    }

    BuiltInFunctionValue(GC::CollectedHeap& heap, int t) : AbstractFunctionValue(heap) {
      heap.increaseSize(size());
      type = static_cast<BuiltInFunctionType>(t);
    }

    ~BuiltInFunctionValue() {
      #ifdef DEBUG
      cout << "DELETING BuiltinFunctionValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    virtual size_t _size() {
      return sizeof(BuiltInFunctionValue);
    }

    virtual void markChildren() {}

    Value call(std::vector<Value> & arguments);
  };
}
