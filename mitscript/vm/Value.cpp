#include "Value.h"
#include "Interpreter.h"
#include "globals.h"
#include "../ir/OptimizingCompiler.h"
#include "../asm/Compiler.h"
#include <list>

#define LRU_SIZE 2

namespace VM {

  Value BareFunctionValue::call(std::vector<Value> & arguments) {
    throw RuntimeException("call on a BareFunctionValue");
  }

  Value ClosureFunctionValue::call(std::vector<Value> & arguments) {
    if (value->parameter_count_ != arguments.size()) {
        throw RuntimeException("An incorrect number of parameters was passed to the function");
    }

    std::vector<Value> local_vars(value->local_vars_.size(), Value::makeNone());
    std::vector<ReferenceValue*> local_reference_vars;
    for (auto var : value->local_reference_vars_) {
      local_reference_vars.push_back(heap.allocate<ReferenceValue>(Value::makeNone()));
    }
    for (auto var : references) {
      local_reference_vars.push_back(var);
    }

    {
      std::map<std::string, int> reverse_index;
      for (int i = 0; i < value->local_reference_vars_.size(); i++) {
        reverse_index[value->local_reference_vars_[i]] = i;
      }
      for (int i = 0; i < arguments.size(); i++) {
        std::string var_name = value->local_vars_[i];
        #if DEBUG
        std::cout << var_name << " = " << arguments[i].toString() << std::endl;
        #endif
        if (reverse_index.count(var_name) == 0) {
            local_vars[i] = arguments[i];
        } else {
            local_reference_vars[reverse_index[var_name]]->value = arguments[i];
        }
      }
    }

    if (has_optimization(OPTIMIZATION_MACHINE_CODE) && !is_compiled) {
      InstructionList ir;
      IR::OptimizingCompiler ir_compiler(value, ir);
      size_t temp_count = ir_compiler.compile(has_optimization(OPTIMIZATION_OPTIMIZATION_PASSES));
      ASM::Compiler asm_compiler(ir, temp_count);
      asm_compiler.compileInto(compiled_func);
      is_compiled = !has_optimization(OPTIMIZATION_COMPILE_ONLY);
    }

    interpreter->push_frame(&local_vars, &local_reference_vars);

    Value result;
    if (is_compiled) {
      result = Value(compiled_func.call<uint64_t, void*, void*, void*>(this, &local_vars[0], &local_reference_vars[0]));
    } else {
      result = interpreter->run_function(this, &local_vars[0], &local_reference_vars[0]);
    }

    interpreter->pop_frame();

    return result;
  }

  Value BuiltInFunctionValue::call(std::vector<Value> & arguments) {
    switch (type) {
        case BuiltInFunctionType::Print: {
          if (arguments.size() != 1) {
            throw RuntimeException("Wrong number of arguments to print");
          }
          #if DEBUG
          std::cout << "===== ";
          #endif
          if (!has_option(OPTION_SHOW_MEMORY_TRACE)) {
            std::cout << arguments[0].toString() << std::endl;
          }
          return Value::makeNone();
        }
        break;

        case BuiltInFunctionType::Input: {
          if (arguments.size() != 0) {
            throw RuntimeException("Wrong number of arguments to input");
          }
          std::string input;
          std::cin >> input;
          return Value::makeString(heap.allocate<StringValue>(input));
        }
        break;

        case BuiltInFunctionType::Intcast: {
          if (arguments.size() != 1) {
            throw RuntimeException("Wrong number of arguments to intcast");
          }
          if (!arguments[0].isString()) {
            throw IllegalCastException("A string wasn't passed to intcast");
          }
          try {
            return Value::makeInteger(std::stoi(arguments[0].toString()));
          } catch (std::invalid_argument& ex) {
            throw IllegalCastException("string passed to intcast doesn't represent int");
          } catch (std::out_of_range& ex) {
            throw RuntimeException("the integer to be casted is too large");
          }
        }
        break;
    }
    throw RuntimeException("Reached end of builtin function execution");
  }
}
