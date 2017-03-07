#pragma once

#include <string>
#include <exception>
#include "Value.h"

class InterpreterException : public std::exception {
private:
  std::string _msg;

public:
  InterpreterException(const std::string& msg) : _msg(msg) {}

  virtual const char* what() const throw() {
    return _msg.c_str();
  }
};

class UninitializedVariableException : public InterpreterException {
  using InterpreterException::InterpreterException;
};

class IllegalCastException : public InterpreterException {
public:
  IllegalCastException(Value *val) : InterpreterException(val->toString()) {}
};

class IllegalArithmeticException : public InterpreterException {
public:
  IllegalArithmeticException() : InterpreterException("divide by zero") {}
};

class RuntimeException : public InterpreterException {
  using InterpreterException::InterpreterException;
};
