#include "Interpreter.h"
#include <algorithm>

using namespace BC;
using namespace GC;

#define COLLECTION_RATIO 0.5

namespace VM {
  Interpreter::Interpreter(Function* const & main_func, size_t max_size) : heap(max_size) {
      main_function = main_func;
  }

  int Interpreter::interpret() {
    Value* val = run_function(*main_function, std::vector<Value*>(), std::vector<ReferenceValue*>());
    if (IntegerValue* i = dynamic_cast<IntegerValue*>(val)) {
      return i->value;
    } else {
      return -1;
    }
  };

  void Interpreter::push_frame(ValueMap* local,
                               std::map<std::string, ReferenceValue*>* local_reference,
                               std::stack<Value*>* local_stack) {
    local_variable_stack.push_back(local);
    local_reference_variable_stack.push_back(local_reference);
    operand_stack_stack.push_back(local_stack);
  }

  void Interpreter::pop_frame() {
    local_variable_stack.pop_back();
    local_reference_variable_stack.pop_back();
    operand_stack_stack.pop_back();
  }

  bool Interpreter::is_top_level() {
    if (local_variable_stack.size() == 0) {
      throw RuntimeException("Cannot call is_top_level, no function is executing");
    }
    return local_variable_stack.size() == 1;
  }

  void Interpreter::potentially_garbage_collect() {
    #ifdef DEBUG
    std::cout << "$$$ Num objects: " << heap.getCount() << std::endl;
    std::cout << "$$$ Bytes current: " << heap.bytes_current << std::endl;
    std::cout << "$$$ Bytes max: " << heap.bytes_max << std::endl;
    #endif
    if (heap.bytes_current >= heap.bytes_max * COLLECTION_RATIO) {
      #ifdef DEBUG
      std::cout << "$$$$$ Building roots..." << std::endl;
      #endif
      std::vector<Value*> roots;
      for (auto local_variables : local_variable_stack) {
        for (auto keyvalue : *local_variables) {
          roots.push_back(keyvalue.second);
        }
      }
      for (auto local_reference_variables : local_reference_variable_stack) {
        for (auto keyvalue : *local_reference_variables) {
          roots.push_back(keyvalue.second);
        }
      }
      for (auto local_stack : operand_stack_stack) {
        std::vector<Value*> stack_holder;
        while (!local_stack->empty()) {
            auto v = local_stack->top();
            roots.push_back(v);
            stack_holder.insert(stack_holder.begin(), v);
            local_stack->pop();
        }
        for (auto v : stack_holder) {
            local_stack->push(v);
        }
      }
      for (auto keyvalue : global_variables) {
        roots.push_back(keyvalue.second);
      }
      #ifdef DEBUG
      std::cout << "$$$$$ Collecting garbage..." << std::endl;
      #endif
      heap.gc(roots.begin(), roots.end());
    }
  };

  static Value* constant_to_value(CollectedHeap& heap, std::shared_ptr<Constant> constant) {
      if (std::shared_ptr<None> c = std::dynamic_pointer_cast<None>(constant)) {
          return heap.allocate<NoneValue>();
      }
      if (std::shared_ptr<Integer> c = std::dynamic_pointer_cast<Integer>(constant)) {
          return heap.allocate<IntegerValue>(c->value);
      }
      if (std::shared_ptr<String> c = std::dynamic_pointer_cast<String>(constant)) {
          return heap.allocate<StringValue>(c->value);
      }
      if (std::shared_ptr<Boolean> c = std::dynamic_pointer_cast<Boolean>(constant)) {
          return heap.allocate<BooleanValue>(c->value);
      }
      throw RuntimeException("Tried to convert a Function to a Value in constant_to_value (usually called from LoadConst) - should be done in LoadFunc");
  }

  template<typename Input, typename Output, typename F>
  static Value* binary_op(CollectedHeap& heap, Value* _a, Value* _b, F func) {
      Input* a = force_cast<Input>(_a);
      Input* b = force_cast<Input>(_b);
      return heap.allocate<Output>(func(a->value, b->value));
  }

  template<typename T, typename F>
  static Value* unary_op(CollectedHeap& heap, Value* _a, F func) {
      T* a = force_cast<T>(_a);
      return heap.allocate<T>(func(a->value));
  }

  template<typename T>
  static T safe_index(const std::vector<T> &v, int i) {
      if (i >= 0 && i < v.size()) {
          return v[i];
      }
      throw RuntimeException("Tried to access an index out of bounds.");
  }

  template<typename T>
  static T safe_pop(std::stack<T> &s) {
      if (!s.empty()) {
          T result = s.top();
          s.pop();
          return result;
      }
      throw InsufficentStackException("Can't pop any more elements off the stack!");
  }

  template<typename T>
  static T safe_peek(std::stack<T> &s) {
      if (!s.empty()) {
          return s.top();
      }
      throw InsufficentStackException("Can't peek off an empty stack!");
  }

  static void print_stack(std::stack<Value*> & stack) {
      std::vector<Value*> stack_holder;
      while (!stack.empty()) {
          auto v = stack.top();
          std::cout << v->toString() << std::endl;
          stack_holder.insert(stack_holder.begin(), v);
          stack.pop();
      }
      for (auto v : stack_holder) {
          stack.push(v);
      }
  };

  std::string getVarFromRefIndex(Function const & func, int index) {
    if (index < 0) { throw RuntimeException("Index out of bounds."); }
    if (index < func.local_reference_vars_.size()) {
        return safe_index(func.local_reference_vars_, index);
    } else {
        index -= func.local_reference_vars_.size();
        return safe_index(func.free_vars_, index);
    }
  }

  Value* Interpreter::run_function(
      Function const & func,
      std::vector<Value*> const & arguments,
      std::vector<ReferenceValue*> const & references
  ) {
      std::stack<Value*> stack;
      ValueMap local_variables;
      std::map<std::string, ReferenceValue*> local_reference_vars;
      push_frame(&local_variables, &local_reference_vars, &stack);

      if (func.parameter_count_ != arguments.size()) {
          throw RuntimeException("An incorrect number of parameters was passed to the function");
      }

      for (auto var : func.local_reference_vars_) {
          local_reference_vars[var] = heap.allocate<ReferenceValue>(var, heap.allocate<NoneValue>());
      }
      for (auto var : func.local_vars_) {
          if (local_reference_vars.count(var) == 0) {
              local_variables[var] = heap.allocate<NoneValue>();
          }
      }
      for (auto reference_value : references) {
          local_reference_vars[reference_value->name] = reference_value;
      }
      for (int i = 0; i < arguments.size(); i++) {
          std::string var_name = func.local_vars_[i];
          #if DEBUG
          std::cout << var_name << " = " << arguments[i]->toString() << std::endl;
          #endif
          if (local_reference_vars.count(var_name) == 0) {
              local_variables[var_name] = arguments[i];
          } else {
              local_reference_vars[var_name]->value = arguments[i];
          }
      }

      int ip = 0;
      while (ip >= 0 && ip < func.instructions.size()) {
          Instruction instruction = func.instructions[ip];
          int ip_increment = 1;
          #if DEBUG
          std::cout << "ip: " << ip << std::endl;
          std::cout << "Instruction: " << instruction.toString() << std::endl;
          #endif
          switch (instruction.operation) {
              // Description: push a constant onto the operand stack
              // Operand 0: index of constant in enclosing function's list of constants
              // Mnemonic:  load_const i
              // Stack:      S => f.constants()[i] :: S
              case Operation::LoadConst: {
                  stack.push(constant_to_value(heap, safe_index(func.constants_, instruction.operand0.value())));
              }
              break;

              // Description: push a  function onto the operand stack
              // Operand 0: index of function  in enclosing function's list of functions
              // Mnemonic:  load_func i
              // Stack:      S => f.functions()[i] :: S
              case Operation::LoadFunc: {
                  int index = instruction.operand0.value();
                  std::shared_ptr<Function> function = safe_index(func.functions_, instruction.operand0.value());
                  if (is_top_level() && index >= 0 && index < static_cast<int>(BuiltInFunctionType::MAX)) {
                      stack.push(heap.allocate<BuiltInFunctionValue>(index));
                  } else {
                      stack.push(heap.allocate<BareFunctionValue>(function));
                  }
              }
              break;

              // Description: load value of local variable and push onto operand stack
              // Operand 0: index of local variable to read
              // Mnemonic: load_local i
              // S => S :: value_of(f.local_vars[i])
              case Operation::LoadLocal: {
                  std::string var_name = safe_index(func.local_vars_, instruction.operand0.value());
                  stack.push(local_variables.at(var_name));
              }
              break;

              // Description: store top of operand stack into local variable
              // Operand 0: index of local variable to store into
              // Operand 1: value to store
              // Mnemonic:  store_local i
              // Stack:     S :: operand 1==> S
              case Operation::StoreLocal: {
                  std::string var_name = safe_index(func.local_vars_, instruction.operand0.value());
                  local_variables[var_name] = safe_pop(stack);
              }
              break;

              // Description: load value of global variable
              // Operand 0: index of name of global variable in enclosing function's names list
              // Mnemonic:  load_global i
              // Stack:     S => global_value_of(f.names[i]) :: S
              case Operation::LoadGlobal: {
                  std::string var_name = safe_index(func.names_, instruction.operand0.value());
                  if (global_variables.count(var_name) == 0) {
                      throw UninitializedVariableException(var_name + " has not been assigned yet, but it has been used");
                  }
                  stack.push(global_variables.at(var_name));
              }
              break;

              // Description: store value into global variable
              // Operand 0: index of name of global variable in enclosing function's names list
              // Operand 1: value to store
              // Mnemonic:  store_global i
              // Stack:     S :: operand 1 ==> S
              case Operation::StoreGlobal: {
                  std::string var_name = safe_index(func.names_, instruction.operand0.value());
                  global_variables[var_name] = safe_pop(stack);
              }
              break;

              // Description: push a reference to a local variable or free variable reference onto the operand stack
              // Operand 0: index of local variable reference
              // Mnemonic:  push_ref i
              // Stack:     S ==>  address_of(var) :: S
              //            wehere var = i < f.local_reference_vars.size() ? f.local_reference_vars[i]
              //                                                           :  f.free_vars[i - f.local_reference_vars.size()]
              case Operation::PushReference:{
                  int index = instruction.operand0.value();
                  stack.push(local_reference_vars[getVarFromRefIndex(func, index)]);
              }
              break;

              // Description: loads the value of a reference onto the operand stack
              // Operand 0: index of local variable reference
              // Mnemonic:  load_ref
              // Stack:     S :: operand 1 => S :: value_of(operand 1)
              case Operation::LoadReference: {
                  int index = instruction.operand0.value();
                  ReferenceValue* rv = local_reference_vars[getVarFromRefIndex(func, index)];
                  stack.push(rv->value);
              }
              break;

              // Description: loads the value of a reference onto the operand stack
              // Operand 0: reference to load from
              // Operand 1: value to store
              // Mnemonic:  load_ref
              // Stack:     S :: operand 2 :: operand 1 => S
              case Operation::StoreReference: {
                  Value* value = safe_pop(stack);
                  int index = instruction.operand0.value();
                  ReferenceValue* rv = local_reference_vars[getVarFromRefIndex(func, index)];
                  rv->value = value;
              }
              break;

              // Description: allocates a record and pushes it on the operand stack
              // Operand 0: N/A
              // Mnemonic:  alloc_record
              // Stack:     S => S :: record
              case Operation::AllocRecord: {
                  stack.push(heap.allocate<RecordValue>());
              }
              break;

              // Description: load value of field from record
              // Operand 0: index of the field's name within the enclosing function's names list
              // Operand 1: record from which to load
              // Mnemonic: field_load i
              // Stack:     S :: operand 1 => S :: record_value_of(operand, f.names[i])
              case Operation::FieldLoad: {
                  std::string var_name = safe_index(func.names_, instruction.operand0.value());
                  RecordValue* rv = dynamic_cast<RecordValue*>(safe_pop(stack));
                  if (!rv) {
                      throw IllegalCastException("Couldn't cast the top of the stack to a record");
                  }
                  stack.push(rv->get(var_name));
              }
              break;

              // Description: store value into field of record
              // Operand 0: index of the field's name within the enclosing function's names list
              // Operand 1: the value to store
              // Operand 2: the record to store into
              // Mnemonic: field_store i
              // Stack:    S :: operand 2 :: operand 1 => S
              case Operation::FieldStore: {
                  std::string var_name = safe_index(func.names_, instruction.operand0.value());
                  Value* stored_value = safe_pop(stack);
                  RecordValue* rv = dynamic_cast<RecordValue*>(safe_pop(stack));
                  if (!rv) {
                      throw IllegalCastException("Couldn't cast the top of the stack to a record");
                  }
                  rv->insert(var_name, stored_value);
              }
              break;

              // Description: load value from index of record
              // Operand 0: N/A
              // Operand 1: the index to read from (can be arbitrary value. indexing adheres to semantics of Assignment #2)
              // Operand 2: the record to read from
              // Stack:     S :: operand 2 :: operand 1 => S
              case Operation::IndexLoad: {
                  Value* index_value = safe_pop(stack);
                  std::string var_name = index_value->toString();
                  RecordValue* rv = dynamic_cast<RecordValue*>(safe_pop(stack));
                  if (!rv) {
                      throw IllegalCastException("Couldn't cast the top of the stack to a record");
                  }
                  stack.push(rv->get(var_name));
              }
              break;

              // Description: store value into index of record
              // Operand 0: N/A
              // Operand 1: the value to store
              // Operand 2: the index to store to (can be arbitrary value. indexing adheres to semantics of Assignment #2)
              // Operand 3: the record to store into
              // Stack:     S :: operand 3 :: operand 2 :: operand 1 => S
              case Operation::IndexStore: {
                  Value* stored_value = safe_pop(stack);
                  Value* index_value = safe_pop(stack);
                  std::string var_name = index_value->toString();
                  RecordValue* rv = dynamic_cast<RecordValue*>(safe_pop(stack));
                  if (!rv) {
                      throw IllegalCastException("Couldn't cast the top of the stack to a record");
                  }
                  rv->insert(var_name, stored_value);
              }
              break;

              // Description: allocate a closure
              // Operand 0:       the number of free variable references passed to the closure
              // Operand 1:       function
              // Operand 2:  - N: references to the function's free variables
              // Mnemonic:   alloc_closure
              // Stack:      S :: operand n :: ... :: operand 3 :: operand 2 :: operand 1 => S :: closure
              case Operation::AllocClosure: {
                  BareFunctionValue* function = dynamic_cast<BareFunctionValue*>(safe_pop(stack));
                  if (!function) {
                      throw RuntimeException("Top of stack wasn't a bare function");
                  }
                  int32_t num_vars = instruction.operand0.value();
                  ClosureFunctionValue* closure = heap.allocate<ClosureFunctionValue>(function->value);
                  for (int i = 0; i < num_vars; i++) {
                      ReferenceValue* reference_value = dynamic_cast<ReferenceValue*>(safe_pop(stack));
                      if (!reference_value) {
                          throw RuntimeException("Error creating closure - value on stack wasn't a reference value");
                      }
                      closure->add_reference(reference_value);
                  }
                  stack.push(closure);
              }
              break;

              // Description: call a closure
              // Operand 0:     number of arguments
              // Operand 1:     closure to call (closure reference)
              // Operand 2 - N: argument ((N - 2) - i)
              // Mnemonic:      call
              // Stack:         S::operand n :: .. :: operand 3 :: operand 2 :: operand 1 => S :: value
              case Operation::Call: {
                  auto value = safe_pop(stack);
                  AbstractFunctionValue* function = dynamic_cast<AbstractFunctionValue*>(value);
                  if (!function) {
                      throw IllegalCastException(value->toString());
                  }
                  int32_t num_args = instruction.operand0.value();
                  std::vector<Value*> arguments;
                  for (int i = 0; i < num_args; i++) {
                      Value* value = safe_pop(stack);
                      ReferenceValue* reference_value = dynamic_cast<ReferenceValue*>(value);
                      if (reference_value) {
                          throw RuntimeException("Error calling function - value on stack was a reference value");
                      }
                      arguments.push_back(value);
                  }
                  std::reverse(arguments.begin(), arguments.end());
                  stack.push(function->call(*this, arguments));
              }
              break;

              // Description: ends the execution of the enclosing function and returns the top of the stack
              // Operand 0:   N/A
              // Operand 1:   value to return
              // Mnemonic:    return
              // Stack::      S :: operand 1 => S
              case Operation::Return: {
                  pop_frame();
                  return safe_peek(stack);
              }
              break;

              // Description: implements addition (as given in the semantics of Assignment #2)
              // Operand 0: N/A
              // Operand 1: right value
              // Operand 2: left value
              // Result:    value of the operation as specified by the semantics of Assignment #2
              // Mnemonic:  sub/mul/div
              // Stack:     S:: operand 2 :: operand 1 => S :: op(operand 2, operand 1)

              case Operation::Add: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  if (can_cast<StringValue>(operand_1) || can_cast<StringValue>(operand_2)) {
                      stack.push(heap.allocate<StringValue>(operand_2->toString() + operand_1->toString()));
                  } else {
                      IntegerValue* operand_1_integer = dynamic_cast<IntegerValue*>(operand_1);
                      IntegerValue* operand_2_integer = dynamic_cast<IntegerValue*>(operand_2);
                      if (operand_1_integer == nullptr || operand_2_integer == nullptr) {
                          throw IllegalCastException("Can't perform addition");
                      }
                      stack.push(heap.allocate<IntegerValue>(operand_2_integer->value + operand_1_integer->value));
                  }
              }
              break;

              // Description: performs an arithmetic operation on two integer operands
              // Operand 0: N/A
              // Operand 1: right value
              // Operand 2: left value
              // Mnemonic:  sub/mul/div
              // Stack:     S:: operand 2 :: operand 1 => S :: op(operand 2, operand 1)


              // Description: computes a comparison operation on two integer operands
              // Operand 0: N/A
              // Operand 1: right value
              // Operand 2: left value
              // Mnemonic:  gt/geq
              // Stack:     S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
              case Operation::Gt: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<IntegerValue, BooleanValue>(heap, operand_2, operand_1, [] (int a, int b) { return a > b; }));
              }
              break;

              case Operation::Geq: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<IntegerValue, BooleanValue>(heap, operand_2, operand_1, [] (int a, int b) { return a >= b; }));
              }
              break;

              case Operation::Sub: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<IntegerValue, IntegerValue>(heap, operand_2, operand_1, [] (int a, int b) { return a - b; }));
              }
              break;

              case Operation::Mul: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<IntegerValue, IntegerValue>(heap, operand_2, operand_1, [] (int a, int b) { return a * b; }));
              }
              break;

              case Operation::Div: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<IntegerValue, IntegerValue>(heap, operand_2, operand_1, [] (int a, int b) {
                      if (b == 0) {
                          throw IllegalArithmeticException("divide by zero");
                      }
                      return a / b;
                  }));
              }
              break;

              // Description: computes the unary minus of the integer operation
              // Operand 0: N/A
              // Operand 1: value
              // Mnemonic:  neg
              // Stack:     S :: operand 1 => S:: - operand 1
              case Operation::Neg: {
                  stack.push(unary_op<IntegerValue>(heap, safe_pop(stack), [] (int a) { return -a; }));
              }
              break;


              // Description: computes an equality between two values (semantics according to Assignment #2)
              // Operand 0: N/A
              // Operand 1: right value
              // Operand 2: left value
              // Mnemonic:  gt/geq
              // Stack:     S :: operand 2 :: operand 1 => S:: eq(operand 2, operand 1)
              case Operation::Eq: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(heap.allocate<BooleanValue>(*operand_2 == *operand_1));
              }
              break;

              // Description: computes a boolean operation on two boolean operands
              // Operand 0: N/A
              // Operand 1: right value
              // Operand 2: left value
              // Mnemonic:  and/or
              // Stack:     S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
              case Operation::And: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<BooleanValue, BooleanValue>(heap, operand_2, operand_1, [] (bool a, bool b) { return a && b; }));
              }
              break;

              case Operation::Or: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(binary_op<BooleanValue, BooleanValue>(heap, operand_2, operand_1, [] (bool a, bool b) { return a || b; }));
              }
              break;

              // Description: computes the logical negation of a boolean operand
              // Operand 0: N/A
              // Operand 1: value
              // Mnemonic:  and/or
              // Stack:     S :: operand 1 => S:: op(operand 1)
              case Operation::Not: {
                  stack.push(unary_op<BooleanValue>(heap, safe_pop(stack), [] (bool a) { return !a; }));
              }
              break;


              // Description: transfers execution of the function to a new instruction offset within the current function
              // Example: goto 0 jumps to the current instruction. goto 1 jumps to the following instruction. goto -1 jumps to the preceeding instruction
              // Operand 0: offset relative to the current instruction offset to jump to.
              // Mnemonic:  goto i
              // Stack:     S => S
              case Operation::Goto:
                  ip_increment = instruction.operand0.value();
              break;

              // Description: transfers execution of the function to a new instruction offset within the current function if the operand evaluates to true
              // Operand 0: offset relative to the current instruction offset to jump to.
              // Operand 1: value
              // Mnemonic:  if i
              // Stack:     S :: operand 1 => S
              case Operation::If: {
                  BooleanValue* operand_1 = dynamic_cast<BooleanValue*>(safe_pop(stack));
                  if (!operand_1) {
                      throw IllegalCastException("Can't cast value to boolean for if condition");
                  }
                  if (operand_1->value) {
                      ip_increment = instruction.operand0.value();
                  }
              }
              break;

              // Description: duplicates the element at the top of the stack.
              // If this element is a reference to a record, function, or local variable, the operation only depulicates the reference
              // Operand 0: N/A
              // Operand 1: value
              // Mnemonic:  dup
              // Stack:     S :: operand 1 => S :: operand 1 :: operand 1
              case Operation::Dup:
                  stack.push(safe_peek(stack));
              break;

              // Description: swaps the two values at the top of the stack
              // Operand 0: N/A
              // Operand 1: a value
              // Operand 2: a value
              // Mnemonic:  swap
              // Stack:     S :: operand 2 :: operand 1 => S :: operand 1 :: operand 2
              case Operation::Swap: {
                  Value* operand_1 = safe_pop(stack);
                  Value* operand_2 = safe_pop(stack);
                  stack.push(operand_1);
                  stack.push(operand_2);
              }
              break;

              // Description: pops and discards the top of the stack
              // Operand 0: N/A
              // Operand 1: a value
              // Mnemonic:  swap
              // Stack:     S :: operand 1 => S
              case Operation::Pop:
                  safe_pop(stack);
              break;

              default:
                  throw RuntimeException("Found an unknown opcode.");
              break;
          };
          potentially_garbage_collect();
          #if DEBUG
          std::cout << "Stack:" << std::endl;
          print_stack(stack);
          std::cout << "----------" << std::endl;
          #endif
          ip += ip_increment;
      }
      pop_frame();
      return heap.allocate<NoneValue>();
  }
}
