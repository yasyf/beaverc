#pragma once

#include "../Exception.h"

namespace BC {
  class CompilerException : public SystemException {
    using SystemException::SystemException;
  };

  class UninitializedVariableException : public CompilerException {
    using CompilerException::CompilerException;

    string description() const override {
      return "UninitializedVariableException";
    }
  };
}
