#pragma once

#include <string>
#include <vector>
#include <map>
#include <stack>
#include "../bccompiler/Types.h"
#include "../gc/CollectedHeap.h"

namespace VM {
  struct Value;
  struct ReferenceValue;

  typedef std::map<std::string, std::shared_ptr<Value>> ValueMap;

  struct Interpreter {
      std::shared_ptr<BC::Function> main_function;
      ValueMap global_variables;
      std::vector<ValueMap*> local_variable_stack;
      std::vector<std::map<std::string, std::shared_ptr<ReferenceValue>>*> local_reference_variable_stack;
      GC::CollectedHeap heap;
      Interpreter(std::shared_ptr<BC::Function> const & main_func, size_t max_size);
      int interpret();
      std::shared_ptr<Value> run_function(BC::Function const & func,
                                          std::vector<std::shared_ptr<Value>> const & arguments,
                                          std::vector<std::shared_ptr<ReferenceValue>> const & references);
      void push_frame(ValueMap* local, std::map<std::string, std::shared_ptr<ReferenceValue>>* local_reference);
      void pop_frame();
      bool is_top_level();
  };
}

#include "Value.h"
