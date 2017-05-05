#pragma once

#include "../Exception.h"

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
}
