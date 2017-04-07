#pragma once

#include <string>
#include "../Exception.h"
#include "Value.h"

using namespace std;

namespace BasicInterpreter {
  class InterpreterException : public SystemException {
    using SystemException::SystemException;
  };

  class UninitializedVariableException : public InterpreterException {
    using InterpreterException::InterpreterException;

    string description() const override {
      return "UninitializedVariableException";
    }
  };

  class IllegalCastException : public InterpreterException {
  public:
    IllegalCastException(Value *val) : InterpreterException(val->toString()) {}

    string description() const override {
      return "IllegalCastException";
    }
  };

  class IllegalArithmeticException : public InterpreterException {
  public:
    using InterpreterException::InterpreterException;

    string description() const override {
      return "IllegalArithmeticException";
    }
  };

  class RuntimeException : public InterpreterException {
    using InterpreterException::InterpreterException;

    string description() const override {
      return "RuntimeException";
    }
  };
}
