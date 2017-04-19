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
#include "Interpreter.h"

namespace VM {
  struct Value;
  struct NoneValue;
  struct BooleanValue;
  struct IntegerValue;
  struct StringValue;
  struct RecordValue;
  struct ReferenceValue;
  struct AbstractFunctionValue;
  struct BareFunctionValue;
  struct ClosureFunctionValue;
  struct BuiltinFunctionValue;

  template<typename T>
  T* force_cast(Value* value) {
    if (T* other = dynamic_cast<T*>(value)) {
      return other;
    }
    throw IllegalCastException("Can't convert to needed type");
  }

  template<typename T>
  bool can_cast(Value* value) {
    if (T* other = dynamic_cast<T*>(value)) {
      return true;
    }
    return false;
  }

  struct Value : public GC::Collectable {
    Value(GC::CollectedHeap& heap) : GC::Collectable(heap) {}
    virtual std::string toString() = 0;
    bool operator==(const Value& other) {
      return typeid(*this) == typeid(other) && equals(other);
    }
  protected:

  private:
    virtual bool equals(const Value& other) = 0;
  };

  struct StringValue : public Value {
    char* memory;
    size_t length;

    StringValue(GC::CollectedHeap& heap, const std::string& value) : Value(heap) {
      length = value.size();
      memory = static_cast<char*>(malloc(length * sizeof(char)));
      strncpy(memory, value.c_str(), length);
      heap.increaseSize(size());
    }

    ~StringValue() {
      #ifdef DEBUG
      cout << "DELETING StringValue: " << toString() << endl;
      #endif
      free(memory);
      heap.decreaseSize(size());
    }

    std::string toString() { return std::string(memory, length); };

    virtual size_t _size() {
      return sizeof(StringValue) + length * sizeof(char);
    }

    virtual void markChildren() {}

    bool equals(const Value& other_) {
      const StringValue& other = dynamic_cast<const StringValue &>(other_);
      return length == other.length && strncmp(memory, other.memory, length) == 0;
    }
  };

  struct BooleanValue : public Value {
    bool value;

    BooleanValue(GC::CollectedHeap& heap, bool value) : Value(heap), value(value) {
      heap.increaseSize(size());
    }

    ~BooleanValue() {
      #ifdef DEBUG
      cout << "DELETING BooleanValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    std::string toString() { return (value) ? "True" : "False"; }

    virtual size_t _size() {
      return sizeof(BooleanValue);
    }

    virtual void markChildren() {}

    bool equals(const Value& other) {
      return value == dynamic_cast<const BooleanValue &>(other).value;
    }
  };

  struct NoneValue : public Value {
    NoneValue(GC::CollectedHeap& heap): Value(heap) {
      heap.increaseSize(size());
    }

    ~NoneValue() {
      #ifdef DEBUG
      cout << "DELETING NoneValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    std::string toString() { return "None"; }

    virtual size_t _size() {
      return sizeof(NoneValue);
    }

    virtual void markChildren() {}

    bool equals(const Value& other) {
      return true;
    }
  };

  struct IntegerValue : public Value {
    int value;

    IntegerValue(GC::CollectedHeap& heap, int value) : Value(heap), value(value) {
      heap.increaseSize(size());
    }

    ~IntegerValue() {
      #ifdef DEBUG
      cout << "DELETING IntegerValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    std::string toString() { return std::to_string(value); }

    virtual size_t _size() {
      return sizeof(IntegerValue);
    }

    virtual void markChildren() { }

    bool equals(const Value& other) {
      return value == dynamic_cast<const IntegerValue &>(other).value;
    }
  };

  struct RecordValue : public Value {
    std::unordered_map<std::string, Value*> values;

    RecordValue(GC::CollectedHeap& heap) : Value(heap) {
      heap.increaseSize(size());
    }

    ~RecordValue() {
      #ifdef DEBUG
      cout << "DELETING RecordValue: " << toString() << endl;
      #endif
      heap.decreaseSize(size());
    }

    void insert(std::string key, Value* inserted) {
      if (values.count(key) == 0)
        heap.increaseSize(sizeof(std::string) + key.capacity() * sizeof(char) + sizeof(Value*));
      values[key] = inserted;
    }

    Value* get(std::string key) {
      if (values.count(key) > 0) {
        return values.at(key);
      }
      return heap.allocate<NoneValue>();
    }

    std::string toString() {
      std::string result = "{";
      for (auto keyvalue : values) {
          result += keyvalue.first + ":" + keyvalue.second->toString() + " ";
      };
      result += "}";
      return result;
    }

    virtual size_t _size() {
      size_t s = sizeof(RecordValue);
      for (auto pair : values) {
        s += sizeof(std::string) + pair.first.capacity() * sizeof(char) + sizeof(Value*);
      }
      return s;
    }

    virtual void markChildren() {
      for (auto pair : values)
        pair.second->mark();
    }

    bool equals(const Value& other) {
      return this == &other;
    }
  };

  struct ReferenceValue : public Value {
    std::string name;
    Value* value;

    ReferenceValue(GC::CollectedHeap& heap, std::string n, Value* v) : Value(heap), name(n), value(v) {
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
        return "ref: " + name;
      #else
        throw RuntimeException("You have uncovered a bug :(");
      #endif
    }

    virtual size_t _size() {
      return sizeof(ReferenceValue) + name.capacity() * sizeof(char);
    }

    virtual void markChildren() {
      value->mark();
    }

    bool equals(const Value& other) {
      throw RuntimeException("Can't check equality with a ReferenceValue");
    }
  };

  struct AbstractFunctionValue : public Value {
    AbstractFunctionValue(GC::CollectedHeap& heap) : Value(heap) {}

    std::string toString() { return "FUNCTION"; };
    virtual Value* call(Interpreter & interpreter, std::vector<Value*> & arguments) = 0;

    bool equals(const Value& o) {
      return this == &o;
    }
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

    Value* call(Interpreter & interpreter, std::vector<Value*> & arguments) {
      return interpreter.run_function(*value, arguments, std::vector<ReferenceValue*>());
    };

    virtual size_t _size() {
      return sizeof(BareFunctionValue);
    }

    virtual void markChildren() {}
  };

  struct ClosureFunctionValue : public AbstractFunctionValue {
    std::shared_ptr<BC::Function> value;
    std::vector<ReferenceValue*> references;

    ClosureFunctionValue(GC::CollectedHeap& heap, std::shared_ptr<BC::Function> value) : AbstractFunctionValue(heap), value(value) {
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

    Value* call(Interpreter & interpreter, std::vector<Value*> & arguments) {
      return interpreter.run_function(*value, arguments, references);
    };

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

    Value* call(Interpreter & interpreter, std::vector<Value*> & arguments) {
      switch (type) {
          case BuiltInFunctionType::Print: {
            if (arguments.size() != 1) {
              throw RuntimeException("Wrong number of arguments to print");
            }
            #if DEBUG
            std::cout << "===== ";
            #endif
            std::cout << arguments[0]->toString() << std::endl;
            return heap.allocate<NoneValue>();
          }
          break;

          case BuiltInFunctionType::Input: {
            if (arguments.size() != 0) {
              throw RuntimeException("Wrong number of arguments to input");
            }
            std::string input;
            std::cin >> input;
            return heap.allocate<StringValue>(input);
          }
          break;

          case BuiltInFunctionType::Intcast: {
            if (arguments.size() != 1) {
              throw RuntimeException("Wrong number of arguments to intcast");
            }
            StringValue* string = force_cast<StringValue>(arguments[0]);
            try {
              return heap.allocate<IntegerValue>(std::stoi(string->toString()));
            } catch (std::invalid_argument& ex) {
              throw IllegalCastException("string passed to intcast doesn't represent int");
            } catch (std::out_of_range& ex) {
              throw RuntimeException("the integer to be casted is too large");
            }
          }
          break;
      }
      throw RuntimeException("Reached end of builtin function execution");
    };
  };
}
