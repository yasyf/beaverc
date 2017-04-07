#pragma once

#include "../Exception.h"

using namespace std;

namespace VM {
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
    using InterpreterException::InterpreterException;

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

  class InsufficentStackException : public InterpreterException {
    using InterpreterException::InterpreterException;

    string description() const override {
      return "InsufficentStackException";
    }
  };
}
