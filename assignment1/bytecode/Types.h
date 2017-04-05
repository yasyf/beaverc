#pragma once

#include "Instructions.h"

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace BC {
  struct Constant {
    virtual ~Constant() { }
  };

  struct None : public Constant {
    virtual ~None() { }

    bool operator==(const None& other) {
      return true;
    }
  };

  struct Integer : public Constant {
    int32_t value;

    Integer(int64_t value) : value(value) { }
    virtual ~Integer() { }

    bool operator==(const Integer& other) {
      return value == other.value;
    }
  };

  struct String : public Constant {
    std::string value;

    String(std::string value) : value(value) { }
    virtual ~String() { }

    bool operator==(const String& other) {
      return value == other.value;
    }
  };

  struct Boolean : public Constant {
    bool value;

    Boolean(bool value) : value(value) { }
    virtual ~Boolean() { }

    bool operator==(const Boolean& other) {
      return value == other.value;
    }
  };

  struct Function
  {
    // List of functions defined within this function (but not functions defined inside of nested functions)
    std::vector<std::shared_ptr<Function>> functions_;

   // List of constants used by the instructions within this function (but not nested functions)
    std::vector<std::shared_ptr<Constant>> constants_;

   // The number of parameters to the function
    uint32_t parameter_count_;

    // List of local variables
    // The first parameter_count_ variables are the function's parameters
    // in their order as given in the paraemter list
    std::vector<std::string> local_vars_;

    // List of local variables accessed by reference (LocalReference)
    std::vector<std::string> local_reference_vars_;

    // List of the names of non-global and non-local variables accessed by the function
    std::vector<std::string> free_vars_;

    // List of global variable and field names used inside the function
    std::vector<std::string> names_;

    InstructionList instructions;
  };

  class FunctionLinkedList : public enable_shared_from_this<FunctionLinkedList> {
  public:
    shared_ptr<Function> function;
    shared_ptr<FunctionLinkedList> last;
    vector<string> local_reference_vars_;
    vector<string> free_reference_vars_;

    FunctionLinkedList(shared_ptr<Function> function) : function(function), last(nullptr),
      local_reference_vars_(), free_reference_vars_()
    {}

    shared_ptr<FunctionLinkedList> extend(shared_ptr<Function> function) {
      shared_ptr<FunctionLinkedList> fll(new FunctionLinkedList(function));
      fll->last = shared_from_this();
      return fll;
    }

    void reset_reference_vars() {
      local_reference_vars_.clear();
      free_reference_vars_.clear();
    }
  };
}
