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
    PointerValue() {};
    virtual std::string toString() = 0;
  };

  struct Value {
    uint64_t value;

    Value() : value(_NONE_TAG) {};

    Value(uint64_t value) : value(value) {};


    bool isInteger() const {
      return __IS_INTEGER_VALUE(value);
    }

    bool isBoolean() const {
      return __IS_BOOLEAN_VALUE(value);
    }

    bool isNone() const {
      return __IS_NONE_VALUE(value);
    }

    bool isPointer() const {
      return __IS_POINTER_VALUE(value);
    }

    bool isStringValue() const {
      return __IS_STRING_VALUE(value);
    }

    bool isStringConstant() const {
      return __IS_STRING_CONSTANT_VALUE(value);
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

    std::string toString() const;

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

    StringValue(const std::string& value);
    StringValue(const Value l, const Value r);
    ~StringValue();

    std::string toString();
    virtual size_t size();
    virtual void markChildren(uint32_t generation, bool mark_recent_only);
  };

  inline const char* unmarked_pointer(const char* val) {
    return (const char*) (((uintptr_t) val) & ~0x1);
  }

  inline const char* marked_pointer(const char* val) {
    return (const char*) (((uintptr_t) val) | 0x1);
  }

  inline bool is_marked(const char* val) {
    return (bool) (((uintptr_t) val) & 1);
  }

  struct constCharHash {
    size_t operator() (const char *val) const {
      return std::hash<std::string>()(unmarked_pointer(val));
    }
  };

  struct constCharCompare {
    bool operator()(const char *val1, const char *val2) const{
      return unmarked_pointer(val1) == unmarked_pointer(val2) || (val1 && val2 && strcmp(unmarked_pointer(val1), unmarked_pointer(val2)) == 0);
    }
  };

  struct RecordValue : public PointerValue {
    std::unordered_map<const char*, Value, constCharHash, constCharCompare> values;

    RecordValue();
    ~RecordValue();

    Value get(Value key);
    Value get(const char* key);
    void insert(Value key, Value inserted);
    void insert(const char* key, Value inserted);

    std::string toString();
    virtual size_t size();
    virtual void markChildren(uint32_t generation, bool mark_recent_only);
  };

  struct ReferenceValue : public PointerValue {
    Value value;

    ReferenceValue(Value v);
    ~ReferenceValue();

    void write(Value v);

    std::string toString();
    virtual size_t size();
    virtual void markChildren(uint32_t generation, bool mark_recent_only);
  };

  struct AbstractFunctionValue : public PointerValue {
    AbstractFunctionValue() {}

    std::string toString();
    virtual Value call(std::vector<Value> & arguments) = 0;
  };

  struct BareFunctionValue : public AbstractFunctionValue {
    std::shared_ptr<BC::Function> value;

    BareFunctionValue(std::shared_ptr<BC::Function> value);
    ~BareFunctionValue();

    Value call(std::vector<Value> & arguments);
    virtual size_t size() { return sizeof(BareFunctionValue); }
    virtual void markChildren(uint32_t generation, bool mark_recent_only) {}
  };

  struct ClosureFunctionValue : public AbstractFunctionValue {
    std::shared_ptr<BC::Function> value;
    std::vector<ReferenceValue*> references;

    ClosureFunctionValue(std::shared_ptr<BC::Function> value);
    ~ClosureFunctionValue();

    void add_reference(ReferenceValue* reference);

    Value call(std::vector<Value> & arguments);
    virtual size_t size();
    virtual void markChildren(uint32_t generation, bool mark_recent_only);
  };

  enum class BuiltInFunctionType {
    Print = 0,
    Input = 1,
    Intcast = 2,
    MAX
  };

  struct BuiltInFunctionValue : public AbstractFunctionValue {
    BuiltInFunctionType type;

    BuiltInFunctionValue(int t);
    ~BuiltInFunctionValue();

    Value call(std::vector<Value> & arguments);
    virtual size_t size() { return sizeof(BuiltInFunctionValue); }
    virtual void markChildren(uint32_t generation, bool mark_recent_only) {}
  };
}
