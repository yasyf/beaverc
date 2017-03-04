#pragma once

#include <exception>

class InterpreterException : public std::exception {
};

class UninitializedVariableException : public InterpreterException {
};

class IllegalCastException : public InterpreterException {
};

class IllegalArithmeticException : public InterpreterException {
};

class RuntimeException : public InterpreterException {
};
