#pragma once

#include "../Exception.h"

using namespace std;

namespace ASM {
  class CompilerException : public SystemException {
    using SystemException::SystemException;
  };

  class UnexpectedOperation : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "UnexpectedOperation";
    }
  };

 class InvalidNumArgs : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "InvalidNumArgs";
    }
  };

  class RegistersExhausted : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "RegistersExhausted";
    }
  };
}
