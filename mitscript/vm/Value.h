#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <map>
#include "../bccompiler/Types.h"
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

  struct Value {
      virtual std::string toString() = 0;
      bool operator==(const Value& other) {
          return typeid(*this) == typeid(other) && equals(other);
      }
  private:
      virtual bool equals(const Value& other) = 0;
  };

  struct StringValue : public Value {
      std::string value;
      StringValue(std::string value) : value(value) {};
      std::string toString() { return value; };

      bool equals(const Value& other) {
          return value == dynamic_cast<const StringValue &>(other).value;
      }
  };

  struct BooleanValue : public Value {
      bool value;
      BooleanValue(bool value) : value(value) {};
      std::string toString() { return (value) ? "True" : "False"; };

      bool equals(const Value& other) {
          return value == dynamic_cast<const BooleanValue &>(other).value;
      }
  };

  struct NoneValue : public Value {
      std::string toString() { return "None"; }

      bool equals(const Value& other) {
          return true;
      }
  };

  struct IntegerValue : public Value {
      int value;
      IntegerValue(int value) : value(value) {};
      std::string toString() { return std::to_string(value); };

      bool equals(const Value& other) {
          return value == dynamic_cast<const IntegerValue &>(other).value;
      }
  };

  struct RecordValue : public Value {
      std::map<std::string, std::shared_ptr<Value>> values;
      void insert(std::string key, std::shared_ptr<Value> inserted) {
          values[key] = inserted;
      };
      std::shared_ptr<Value> get(std::string key) {
          if (values.count(key) > 0) {
              return values.at(key);
          }
          return std::shared_ptr<Value>(new NoneValue());
      };
      std::string toString() {
          std::string result = "{";
          for (auto keyvalue : values) {
              result += keyvalue.first + ":" + keyvalue.second->toString() + " ";
          };
          result += "}";
          return result;
      };

      bool equals(const Value& other) {
          return this == &other;
      }
  };

  struct ReferenceValue : public Value {
      std::string name;
      std::shared_ptr<ValueMap> location;
      ReferenceValue(std::string n, std::shared_ptr<ValueMap> l) { name = n; location = l; };
      std::string toString() {
        #if DEBUG
          return "ref: " + name;
        #else
          throw RuntimeException("You have uncovered a bug :(");
        #endif
      };

      bool equals(const Value& other) {
        throw RuntimeException("Can't check equality with a ReferenceValue");
      }
  };

  struct AbstractFunctionValue : public Value {
      std::string toString() { return "FUNCTION"; };
      virtual std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) = 0;

      bool equals(const Value& o) {
          return this == &o;
      }
  };

  struct BareFunctionValue : public AbstractFunctionValue {
      std::shared_ptr<BC::Function> value;
      BareFunctionValue(std::shared_ptr<BC::Function> value) : value(value) {};
      std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) {
          return interpreter.run_function(*value, arguments, std::vector<std::shared_ptr<ReferenceValue>>());
      };
  };

  struct ClosureFunctionValue : public AbstractFunctionValue {
      std::shared_ptr<BC::Function> value;
      std::vector<std::shared_ptr<ReferenceValue>> references;
      ClosureFunctionValue(std::shared_ptr<BC::Function> value) : value(value) {};
      void add_reference(std::shared_ptr<ReferenceValue> reference) { references.push_back(reference); };
      std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) {
          return interpreter.run_function(*value, arguments, references);
      };
  };

  enum class BuiltInFunctionType {
      Print = 0,
      Input = 1,
      Intcast = 2,
      MAX
  };

  struct BuiltInFunctionValue : public AbstractFunctionValue {
      BuiltInFunctionType type;
      BuiltInFunctionValue(BuiltInFunctionType type) : type(type) {};
      BuiltInFunctionValue(int t) { type = static_cast<BuiltInFunctionType>(t); };
      std::shared_ptr<Value> call(Interpreter & interpreter, std::vector<std::shared_ptr<Value>> & arguments) {
          switch (type) {
              case BuiltInFunctionType::Print: {
                  if (arguments.size() != 1) {
                      throw RuntimeException("Wrong number of arguments to print");
                  }
                  #ifdef DEBUG
                  std::cout << "===== ";
                  #endif
                  std::cout << arguments[0]->toString() << std::endl;
                  return std::shared_ptr<Value>(new NoneValue());
              }
              break;

              case BuiltInFunctionType::Input: {
                  if (arguments.size() != 0) {
                      throw RuntimeException("Wrong number of arguments to input");
                  }
                  std::string input;
                  std::cin >> input;
                  return std::shared_ptr<Value>(new StringValue(input));
              }
              break;

              case BuiltInFunctionType::Intcast: {
                  if (arguments.size() != 1) {
                      throw RuntimeException("Wrong number of arguments to intcast");
                  }
                  std::shared_ptr<StringValue> string = force_cast<StringValue>(arguments[0]);
                  try {
                      return std::shared_ptr<Value>(new IntegerValue(std::stoi(string->value)));
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
