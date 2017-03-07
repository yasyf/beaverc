#pragma once

#include <string>
#include <exception>
#include "Value.h"

using namespace std;

class InterpreterException : public exception {
private:
  string _msg;

public:
  InterpreterException(const string& msg) : _msg(msg) {}

  virtual string description() const = 0;

  virtual const char* what() const throw() {
    return (new string(description() + ": " + _msg))->c_str();
  }
};

class UninitializedVariableException : public InterpreterException {
  using InterpreterException::InterpreterException;

  string description() const override {
    return "uninitialized variable";
  }
};

class IllegalCastException : public InterpreterException {
public:
  IllegalCastException(Value *val) : InterpreterException(val->toString()) {}

  string description() const override {
    return "illegal cast";
  }
};

class IllegalArithmeticException : public InterpreterException {
public:
  using InterpreterException::InterpreterException;

  string description() const override {
    return "illegal arithmetic";
  }
};

class RuntimeException : public InterpreterException {
  using InterpreterException::InterpreterException;

  string description() const override {
    return "runtime exception";
  }
};
