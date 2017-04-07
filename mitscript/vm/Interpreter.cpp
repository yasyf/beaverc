#include "interpreter.h"

Interpreter::Interpreter(std::shared_ptr<Function> const & main_func) {
    main_function = main_func;
    global_variables.reset();
}

void Interpreter::interpret() {
    run_function(*main_function, std::vector<std::shared_ptr<Value>>(), std::vector<std::shared_ptr<ReferenceValue>>());
};

static std::shared_ptr<Value> constant_to_value(std::shared_ptr<Constant> constant) {
    if (std::shared_ptr<None> c = std::dynamic_pointer_cast<None>(constant)) {
        return std::shared_ptr<Value>(new NoneValue());
    }
    if (std::shared_ptr<Integer> c = std::dynamic_pointer_cast<Integer>(constant)) {
        return std::shared_ptr<Value>(new IntegerValue(c->value));
    }
    if (std::shared_ptr<String> c = std::dynamic_pointer_cast<String>(constant)) {
        return std::shared_ptr<Value>(new StringValue(c->value));
    }
    if (std::shared_ptr<Boolean> c = std::dynamic_pointer_cast<Boolean>(constant)) {
        return std::shared_ptr<Value>(new BooleanValue(c->value));
    }
    throw RuntimeException("Tried to convert a Function to a Value in constant_to_value (usually called from LoadConst) - should be done in LoadFunc");
}

template<typename Input, typename Output, typename F>
static std::shared_ptr<Value> binary_op(std::shared_ptr<Value> _a, std::shared_ptr<Value> _b, F func) {
    std::shared_ptr<Input> a = force_cast<Input>(_a);
    std::shared_ptr<Input> b = force_cast<Input>(_b);
    return std::shared_ptr<Value>(new Output(func(a->value, b->value)));
}

template<typename T, typename F>
static std::shared_ptr<Value> unary_op(std::shared_ptr<Value> _a, F func) {
    std::shared_ptr<T> a = force_cast<T>(_a);
    return std::shared_ptr<Value>(new T(func(a->value)));
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

static void print_stack(std::stack<std::shared_ptr<Value>> & stack) {
    std::vector<std::shared_ptr<Value>> stack_holder;
    while (!stack.empty()) {
        auto v = stack.top();
        std::cout << v->toString() << std::endl;
        stack_holder.insert(stack_holder.begin(), v);
        stack.pop();
    }
    std::cout << "Stack size: " << stack_holder.size() << std::endl;
    for (auto v : stack_holder) {
        stack.push(v);
    }
};

std::shared_ptr<Value> Interpreter::run_function(
    Function const & func,
    std::vector<std::shared_ptr<Value>> const & arguments, 
    std::vector<std::shared_ptr<ReferenceValue>> const & references
) {
    if (func.parameter_count_ != arguments.size()) {
        throw RuntimeException("An incorrect number of parameters was passed to the function");
    }
    std::stack<std::shared_ptr<Value>> stack;
    std::shared_ptr<ValueMap> local_variables = std::shared_ptr<ValueMap>(new ValueMap());
    std::shared_ptr<ValueMap> local_reference_vars;
    std::map<std::string, std::shared_ptr<ValueMap>> reference_locations;
    if (!global_variables) {
        // This is the global function
        global_variables = local_reference_vars = local_variables;
    } else {
        local_reference_vars = std::shared_ptr<ValueMap>(new ValueMap());
        for (auto var : func.local_reference_vars_) {
            (*local_reference_vars)[var] = std::shared_ptr<Value>(new NoneValue());
        }
        for (auto var : func.local_vars_) {
            if (local_reference_vars->count(var) == 0) {
                (*local_variables)[var] = std::shared_ptr<Value>(new NoneValue());
            }
        }
        for (auto reference_value : references) {
            reference_locations[reference_value->name] = reference_value->location;
        }
        for (int i = 0; i < arguments.size(); i++) {
            std::string var_name = func.local_vars_[i];
            if (local_reference_vars->count(var_name) == 0) {
                (*local_variables)[var_name] = arguments[i];
            } else {
                (*local_reference_vars)[var_name] = arguments[i];
            }
        } 
      
    }
    int ip = 0;
    while (ip >= 0 && ip < func.instructions.size()) {
        Instruction instruction = func.instructions[ip];
        int ip_increment = 1;
        #ifdef DEBUG
        std::cout << "instruction: " << ip << std::endl;
        std::cout << "operation: " << static_cast<int>(instruction.operation) << std::endl;
        #endif
        switch (instruction.operation) {
            // Description: push a constant onto the operand stack
            // Operand 0: index of constant in enclosing function's list of constants 
            // Mnemonic:  load_const i
            // Stack:      S => f.constants()[i] :: S 
            case Operation::LoadConst: {
                stack.push(constant_to_value(safe_index(func.constants_, instruction.operand0.value())));
            }
            break;

            // Description: push a  function onto the operand stack
            // Operand 0: index of function  in enclosing function's list of functions 
            // Mnemonic:  load_func i
            // Stack:      S => f.functions()[i] :: S 
            case Operation::LoadFunc: {
                int index = instruction.operand0.value();
                std::shared_ptr<Function> function = safe_index(func.functions_, instruction.operand0.value());
                if (global_variables == local_reference_vars && // This means we're top level
                    index >= 0 && index < static_cast<int>(BuiltInFunctionType::MAX)) { 
                    stack.push(std::shared_ptr<Value>(new BuiltInFunctionValue(index)));
                } else {
                    stack.push(std::shared_ptr<Value>(new BareFunctionValue(function)));
                }
            }
            break;
           
            // Description: load value of local variable and push onto operand stack
            // Operand 0: index of local variable to read
            // Mnemonic: load_local i
            // S => S :: value_of(f.local_vars[i]) 
            case Operation::LoadLocal: {
                std::string var_name = safe_index(func.local_vars_, instruction.operand0.value());
                if (local_reference_vars->count(var_name) == 0) {
                    if (local_variables->count(var_name) == 0) {
                        throw UninitializedVariableException(var_name + " has not been assigned yet, but it has been used");
                    }
                    stack.push(local_variables->at(var_name));
                } else {
                    stack.push(local_reference_vars->at(var_name));
                }
            }
            break;

            // Description: store top of operand stack into local variable
            // Operand 0: index of local variable to store into
            // Operand 1: value to store
            // Mnemonic:  store_local i
            // Stack:     S :: operand 1==> S
            case Operation::StoreLocal: {
                std::string var_name = safe_index(func.local_vars_, instruction.operand0.value());
                if (local_reference_vars->count(var_name) == 0) {
                    (*local_variables)[var_name] = safe_pop(stack);
                } else {
                    (*local_reference_vars)[var_name] = safe_pop(stack);
                }
            }
            break;

            // Description: load value of global variable
            // Operand 0: index of name of global variable in enclosing function's names list
            // Mnemonic:  load_global i
            // Stack:     S => global_value_of(f.names[i]) :: S
            case Operation::LoadGlobal: {
                std::string var_name = safe_index(func.names_, instruction.operand0.value());
                if (global_variables->count(var_name) == 0) {
                    throw UninitializedVariableException(var_name + " has not been assigned yet, but it has been used");
                }
                stack.push(global_variables->at(var_name));
            }
            break;

            // Description: store value into global variable
            // Operand 0: index of name of global variable in enclosing function's names list
            // Operand 1: value to store
            // Mnemonic:  store_global i
            // Stack:     S :: operand 1 ==> S
            case Operation::StoreGlobal: {
                std::string var_name = safe_index(func.names_, instruction.operand0.value());
                (*global_variables)[var_name] = safe_pop(stack);
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
                if (index < 0) { throw RuntimeException("Index out of bounds."); }
                if (index < func.local_reference_vars_.size()) {
                    std::string var_name = safe_index(func.local_reference_vars_, index);
                    stack.push(std::shared_ptr<Value>(new ReferenceValue(var_name, local_reference_vars)));
                } else {
                    index -= func.local_reference_vars_.size();
                    std::string var_name = safe_index(func.free_vars_, index);
                    stack.push(std::shared_ptr<Value>(new ReferenceValue(var_name, reference_locations[var_name])));
                }
            }
            break;

            // Description: loads the value of a reference onto the operand stack
            // Operand 0: N/A
            // Operand 1: reference to load from
            // Mnemonic:  load_ref
            // Stack:     S :: operand 1 => S :: value_of(operand 1)
            case Operation::LoadReference: {
                std::shared_ptr<ReferenceValue> rv = std::dynamic_pointer_cast<ReferenceValue>(safe_pop(stack));
                if (!rv) {
                    throw RuntimeException("The top of the stack isn't a ReferenceValue");
                }
                if (rv->location->count(rv->name) == 0) {
                    throw UninitializedVariableException("A variable was accessed that has not been initialized");
                }
                stack.push(rv->location->at(rv->name));
            }
            break;

            // Description: loads the value of a reference onto the operand stack
            // Operand 0: N/A
            // Operand 1: value to store
            // Operand 2: reference to store to
            // Mnemonic:  load_ref
            // Stack:     S :: operand 2 :: operand 1 => S
            case Operation::StoreReference: {
                std::shared_ptr<Value> value = safe_pop(stack);
                std::shared_ptr<ReferenceValue> rv = std::dynamic_pointer_cast<ReferenceValue>(safe_pop(stack));
                if (!rv) {
                    throw RuntimeException("The top of the stack isn't a ReferenceValue");
                }
                (*(rv->location))[rv->name] = value;
            }
            break;
            
            // Description: allocates a record and pushes it on the operand stack
            // Operand 0: N/A
            // Mnemonic:  alloc_record
            // Stack:     S => S :: record
            case Operation::AllocRecord: {
                stack.push(std::shared_ptr<Value>(new RecordValue()));
            }
            break;

            // Description: load value of field from record
            // Operand 0: index of the field's name within the enclosing function's names list
            // Operand 1: record from which to load
            // Mnemonic: field_load i
            // Stack:     S :: operand 1 => S :: record_value_of(operand, f.names[i])
            case Operation::FieldLoad: {
                std::string var_name = safe_index(func.names_, instruction.operand0.value());
                std::shared_ptr<RecordValue> rv = std::dynamic_pointer_cast<RecordValue>(safe_pop(stack));
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
                std::shared_ptr<Value> stored_value = safe_pop(stack);
                std::shared_ptr<RecordValue> rv = std::dynamic_pointer_cast<RecordValue>(safe_pop(stack));
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
                std::shared_ptr<Value> index_value = safe_pop(stack);
                std::string var_name = index_value->toString();
                std::shared_ptr<RecordValue> rv = std::dynamic_pointer_cast<RecordValue>(safe_pop(stack));
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
                std::shared_ptr<Value> stored_value = safe_pop(stack);
                std::shared_ptr<Value> index_value = safe_pop(stack);
                std::string var_name = index_value->toString();
                std::shared_ptr<RecordValue> rv = std::dynamic_pointer_cast<RecordValue>(safe_pop(stack));
                if (!rv) {
                    throw IllegalCastException("Couldn't cast the top of the stack to a record");
                }
                rv->insert(var_name, stored_value);
            }
            break;
            
            // Description: allocate a closure
            // Operand 0:       N/A
            // Operand 1:       function
            // Operand 2:       the number of free variable references passed to the closure
            // Operand 3:  - N: references to the function's free variables
            // Mnemonic:   alloc_closure
            // Stack:      S :: operand n :: ... :: operand 3 :: operand 2 :: operand 1 => S :: closure 
            case Operation::AllocClosure: {
                std::shared_ptr<BareFunctionValue> function = std::dynamic_pointer_cast<BareFunctionValue>(safe_pop(stack));
                if (!function) {
                    throw RuntimeException("Top of stack wasn't a bare function");
                }
                std::shared_ptr<IntegerValue> num_vars = std::dynamic_pointer_cast<IntegerValue>(safe_pop(stack));
                if (!num_vars) {
                    throw RuntimeException("Number of variables wasn't an integer");
                }
                std::shared_ptr<ClosureFunctionValue> closure = std::shared_ptr<ClosureFunctionValue>(new ClosureFunctionValue(function->value));
                for (int i = 0; i < num_vars->value; i++) {
                    std::shared_ptr<ReferenceValue> reference_value = std::dynamic_pointer_cast<ReferenceValue>(safe_pop(stack));
                    if (!reference_value) {
                        throw RuntimeException("Error creating closure - value on stack wasn't a reference value");
                    }
                    closure->add_reference(reference_value);
                }
                stack.push(closure);
            }
            break;
           
            // Description: call a closure
            // Operand 0:     N/A
            // Operand 1:     closure to call (closure reference)
            // Operand 2:     number of arguments
            // Operand 3 - N: argument ((N - 3) - i)
            // Mnemonic:      call
            // Stack:         S::operand n :: .. :: operand 3 :: operand 2 :: operand 1 => S :: value
            case Operation::Call: {
                std::shared_ptr<AbstractFunctionValue> function = std::dynamic_pointer_cast<AbstractFunctionValue>(safe_pop(stack));
                if (!function) {
                    throw RuntimeException("Top of stack wasn't a function");
                }
                std::shared_ptr<IntegerValue> num_args = std::dynamic_pointer_cast<IntegerValue>(safe_pop(stack));
                if (!num_args) {
                    throw RuntimeException("Number of arguments wasn't an integer");
                }
                std::vector<std::shared_ptr<Value>> arguments;
                for (int i = 0; i < num_args->value; i++) {
                    std::shared_ptr<Value> value = safe_pop(stack);
                    std::shared_ptr<ReferenceValue> reference_value = std::dynamic_pointer_cast<ReferenceValue>(value);
                    if (reference_value) {
                        throw RuntimeException("Error calling function - value on stack was a reference value");
                    }
                    arguments.push_back(value);
                }
                stack.push(function->call(*this, arguments));
            }
            break;

            // Description: ends the execution of the enclosing function and returns the top of the stack
            // Operand 0:   N/A
            // Operand 1:   value to return
            // Mnemonic:    return
            // Stack::      S :: operand 1 => S
            case Operation::Return: {
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
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                if (can_cast<StringValue>(operand_1) || can_cast<StringValue>(operand_2)) {
                    stack.push(std::shared_ptr<Value>(new StringValue(operand_2->toString() + operand_1->toString())));
                } else {
                    std::shared_ptr<IntegerValue> operand_1_integer = std::dynamic_pointer_cast<IntegerValue>(operand_1);
                    std::shared_ptr<IntegerValue> operand_2_integer = std::dynamic_pointer_cast<IntegerValue>(operand_2);
                    if (operand_1_integer == nullptr || operand_2_integer == nullptr) {
                        throw IllegalCastException("Can't perform addition");
                    }
                    stack.push(std::shared_ptr<Value>(new IntegerValue(operand_2_integer->value + operand_1_integer->value)));
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
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<IntegerValue, BooleanValue>(operand_2, operand_1, [] (int a, int b) { return a > b; }));
            }
            break;

            case Operation::Geq: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<IntegerValue, BooleanValue>(operand_2, operand_1, [] (int a, int b) { return a >= b; }));
            }
            break;

            case Operation::Sub: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<IntegerValue, IntegerValue>(operand_2, operand_1, [] (int a, int b) { return a - b; }));
            }
            break;

            case Operation::Mul: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<IntegerValue, IntegerValue>(operand_2, operand_1, [] (int a, int b) { return a * b; }));
            }
            break;
            
            case Operation::Div: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<IntegerValue, IntegerValue>(operand_2, operand_1, [] (int a, int b) {
                    if (b == 0) {
                        throw IllegalArithmeticException("divide by zero :(");
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
                stack.push(unary_op<IntegerValue>(safe_pop(stack), [] (int a) { return -a; }));
            }
            break;


            // Description: computes an equality between two values (semantics according to Assignment #2)
            // Operand 0: N/A
            // Operand 1: right value 
            // Operand 2: left value
            // Mnemonic:  gt/geq
            // Stack:     S :: operand 2 :: operand 1 => S:: eq(operand 2, operand 1)
            case Operation::Eq: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(std::shared_ptr<Value>(new BooleanValue(*operand_2 == *operand_1)));
            }
            break;

            // Description: computes a boolean operation on two boolean operands
            // Operand 0: N/A
            // Operand 1: right value 
            // Operand 2: left value
            // Mnemonic:  and/or
            // Stack:     S :: operand 2 :: operand 1 => S:: op(operand 2, operand 1)
            case Operation::And: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<BooleanValue, BooleanValue>(operand_2, operand_1, [] (bool a, bool b) { return a && b; }));
            }
            break;

            case Operation::Or: {
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
                stack.push(binary_op<BooleanValue, BooleanValue>(operand_2, operand_1, [] (bool a, bool b) { return a || b; }));
            }
            break;
            
            // Description: computes the logical negation of a boolean operand
            // Operand 0: N/A
            // Operand 1: value
            // Mnemonic:  and/or
            // Stack:     S :: operand 1 => S:: op(operand 1)
            case Operation::Not: {
                stack.push(unary_op<BooleanValue>(safe_pop(stack), [] (bool a) { return !a; }));
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
                std::shared_ptr<BooleanValue> operand_1 = std::dynamic_pointer_cast<BooleanValue>(safe_pop(stack));
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
                std::shared_ptr<Value> operand_1 = safe_pop(stack);
                std::shared_ptr<Value> operand_2 = safe_pop(stack);
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
        #ifdef DEBUG
        print_stack(stack);
        std::cout << "----------" << std::endl;
        #endif
        ip += ip_increment;
    }
    return std::shared_ptr<Value>(new NoneValue());
}