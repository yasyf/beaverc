#pragma once

#include <string>
#include <map>
#include <stack>
#include "../bccompiler/Types.h"

namespace VM {
  struct Value;
  struct ReferenceValue;

  typedef std::map<std::string, std::shared_ptr<Value>> ValueMap;

  struct Interpreter {
      std::shared_ptr<BC::Function> main_function;
      std::shared_ptr<ValueMap> global_variables;
      Interpreter(std::shared_ptr<BC::Function> const & main_func) ;
      int interpret();
      std::shared_ptr<Value> run_function(BC::Function const & func,
                                          std::vector<std::shared_ptr<Value>> const & arguments,
                                          std::vector<std::shared_ptr<ReferenceValue>> const & references);
  };
}

#include "Value.h"
