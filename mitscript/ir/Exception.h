#pragma once

#include "../Exception.h"
#include "../vm/options.h"

namespace IR {
  class CompilerException : public SystemException {
    using SystemException::SystemException;
  };

  class InvalidOperationException : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "InvalidOperationException";
    }
  };

  class IllegalCastException : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "IllegalCastException";
    }
  };

  class IllegalArithmeticException : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "IllegalArithmeticException";
    }
  };

  template<typename E>
  void throw_exception(E ex) {
    if (has_option(OPTION_COMPILE_ERRORS)) {
      throw ex;
    }
  }
}
