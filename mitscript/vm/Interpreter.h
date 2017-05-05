#pragma once

#include <string>
#include <vector>
#include <map>
#include <stack>
#include "../bccompiler/Types.h"
#include "../gc/Collectable.h"
#include "../gc/CollectedHeap.h"
#include "Value.fwd.h"

namespace VM {

  typedef std::map<std::string, Value> ValueMap;

  struct Interpreter {
      ClosureFunctionValue* main_closure;
      ValueMap global_variables;
      std::vector<Value*> local_variable_stack;
      std::vector<int> local_variable_size_stack;
      std::vector<std::vector<ReferenceValue*>*> local_reference_variable_stack;
      std::vector<std::stack<Value>*> operand_stack_stack;
      GC::CollectedHeap heap;
      Interpreter(std::shared_ptr<BC::Function> main_func, size_t max_size);
      int interpret();
      Value run_function(ClosureFunctionValue* closure,
                          std::vector<Value> const & arguments);
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
