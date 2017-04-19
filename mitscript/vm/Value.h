#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>
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
  std::shared_ptr<T> force_cast(std::shared_ptr<Value> value) {
    std::shared_ptr<T> other = std::dynamic_pointer_cast<T>(value);
    if (!other) {
      throw IllegalCastException("Can't convert to needed type");
    }
    return other;
  }

  template<typename T>
  bool can_cast(std::shared_ptr<Value> value) {
    std::shared_ptr<T> other = std::dynamic_pointer_cast<T>(value);
    if (!other) {
      return false;
    }
    return true;
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
    std::string value;

    StringValue(GC::CollectedHeap& heap, std::string value) : Value(heap), value(value) {
      heap.increaseSize(size());
    }

    ~StringValue() {
      heap.decreaseSize(size());
    }

    std::string toString() { return value; };

    virtual size_t size() {
      return sizeof(StringValue) + value.capacity() * sizeof(char);
    }

    virtual void markChildren() {}

    bool equals(const Value& other) {
      return value == dynamic_cast<const StringValue &>(other).value;
    }
  };

  struct BooleanValue : public Value {
    bool value;

    BooleanValue(GC::CollectedHeap& heap, bool value) : Value(heap), value(value) {
      heap.increaseSize(size());
    }

    ~BooleanValue() {
      heap.decreaseSize(size());
    }

    std::string toString() { return (value) ? "True" : "False"; }

    virtual size_t size() {
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
      heap.decreaseSize(size());
    }

    std::string toString() { return "None"; }

    virtual size_t size() {
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
      heap.decreaseSize(size());
    }

    std::string toString() { return std::to_string(value); }

    virtual size_t size() {
      return sizeof(IntegerValue);
    }

    virtual void markChildren() { }

    bool equals(const Value& other) {
      return value == dynamic_cast<const IntegerValue &>(other).value;
    }
  };

  struct RecordValue : public Value {
    std::map<std::string, std::shared_ptr<Value>> values;

    RecordValue(GC::CollectedHeap& heap) : Value(heap) {
      heap.increaseSize(size());
    }

    ~RecordValue() {
      heap.decreaseSize(size());
    }

    void insert(std::string key, std::shared_ptr<Value> inserted) {
      if (values.count(key) == 0)
        heap.increaseSize(sizeof(std::string) + key.capacity() * sizeof(char) + sizeof(std::shared_ptr<Value>));
      values[key] = inserted;
    }

    std::shared_ptr<Value> get(std::string key) {
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

    virtual size_t size() {
      size_t s = sizeof(RecordValue);
      for (auto pair : values) {
        s += sizeof(std::string) + pair.first.capacity() * sizeof(char) + sizeof(std::shared_ptr<Value>);
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
    std::shared_ptr<Value> value;

    ReferenceValue(GC::CollectedHeap& heap, std::string n, std::shared_ptr<Value> v) : Value(heap), name(n), value(v) {
      heap.increaseSize(size());
    }

    ~ReferenceValue() {
      heap.decreaseSize(size());
    }

    std::string toString() {
      #if DEBUG
        return "ref: " + name;
      #else
        throw RuntimeException("You have uncovered a bug :(");
      #endif
    }

    virtual size_t size() {
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
    virtual std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) = 0;

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
      heap.decreaseSize(size());
    }

    std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) {
      return interpreter.run_function(*value, arguments, std::vector<std::shared_ptr<ReferenceValue>>());
    };

    virtual size_t size() {
      return sizeof(BareFunctionValue);
    }

    virtual void markChildren() {}
  };

  struct ClosureFunctionValue : public AbstractFunctionValue {
    std::shared_ptr<BC::Function> value;
    std::vector<std::shared_ptr<ReferenceValue>> references;

    ClosureFunctionValue(GC::CollectedHeap& heap, std::shared_ptr<BC::Function> value) : AbstractFunctionValue(heap), value(value) {
      heap.increaseSize(size());
    }

    ~ClosureFunctionValue() {
      heap.decreaseSize(size());
    }


    void add_reference(std::shared_ptr<ReferenceValue> reference) { references.push_back(reference); };
    std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) {
      return interpreter.run_function(*value, arguments, references);
    };

    virtual size_t size() {
      return sizeof(ClosureFunctionValue) + references.size() * sizeof(std::shared_ptr<ReferenceValue>);
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
      heap.decreaseSize(size());
    }

    virtual size_t size() {
      return sizeof(BuiltInFunctionValue);
    }

    virtual void markChildren() {}

    std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) {
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
            std::shared_ptr<StringValue> string = force_cast<StringValue>(arguments[0]);
            try {
              return heap.allocate<IntegerValue>(std::stoi(string->value));
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
