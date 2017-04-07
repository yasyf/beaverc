#pragma once

#include <string>
#include <exception>

using namespace std;

class SystemException : public exception {
private:
  string _msg;

public:
  SystemException(const string& msg) : _msg(msg) {}

  virtual string description() const = 0;

  virtual const char* what() const throw() {
    return (new string(description() + ": " + _msg))->c_str();
  }
};

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