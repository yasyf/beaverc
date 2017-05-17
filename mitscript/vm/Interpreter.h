#pragma once

#include <string>
#include <vector>
#include <map>
#include <stack>
#include "../bccompiler/Types.h"
#include "../gc/Collectable.h"
#include "../gc/CollectedHeap.h"
#include "Value.fwd.h"
#include "Interpreter.fwd.h"

namespace VM {

  typedef std::map<std::string, Value> ValueMap;

  struct Interpreter {
      ClosureFunctionValue* main_closure;
      ValueMap global_variables;
      std::vector<std::pair<Value*, int>> local_variable_stack;
      std::vector<std::pair<ReferenceValue**, int>> local_reference_variable_stack;
      std::vector<std::stack<Value>*> operand_stack_stack;
      GC::CollectedHeap heap;
      Interpreter(std::shared_ptr<BC::Function> main_func, size_t max_size);
      int interpret();
      Value run_function(ClosureFunctionValue* closure, Value* local_variables, ReferenceValue** local_reference_vars);
      void push_frame(Value* local, int local_length, ReferenceValue** local_reference, int reference_length);
      void pop_frame();
      void push_stack(std::stack<Value>* local_stack);
      void pop_stack();
      bool is_top_level();

      bool will_garbage_collect();
      void potentially_garbage_collect();
  };
}

#include "Value.h"
