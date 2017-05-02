#pragma once

#include <string>
#include <vector>
#include <map>
#include <stack>
#include "../bccompiler/Types.h"
#include "../gc/Collectable.h"
#include "../gc/CollectedHeap.h"

namespace VM {
  struct Value;
  struct ReferenceValue;

  typedef std::map<std::string, Value> ValueMap;

  struct Interpreter {
      BC::Function* main_function;
      ValueMap global_variables;
      std::vector<Value*> local_variable_stack;
      std::vector<int> local_variable_size_stack;
      std::vector<std::vector<ReferenceValue*>*> local_reference_variable_stack;
      std::vector<std::stack<Value>*> operand_stack_stack;
      GC::CollectedHeap heap;
      Interpreter(BC::Function* const & main_func, size_t max_size);
      int interpret();
      Value run_function(BC::Function const & func,
                          std::vector<Value> const & arguments,
                          std::vector<ReferenceValue*> const & references);
      void push_frame(Value* local,
                      int local_size,
                      std::vector<ReferenceValue*>* local_reference,
                      std::stack<Value>* local_stack);
      void pop_frame();
      bool is_top_level();

      void potentially_garbage_collect();
  };
}

#include "Value.h"
