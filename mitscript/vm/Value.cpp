#include "Value.h"
#include "Interpreter.h"
#include "globals.h"
#include "../ir/Compiler.h"
#include "../asm/Compiler.h"
#include <list>

#define LRU_SIZE 2

namespace VM {

  Value BareFunctionValue::call(std::vector<Value> & arguments) {
    throw RuntimeException("call on a BareFunctionValue");
  }

  Value ClosureFunctionValue::call(std::vector<Value> & arguments) {
    std::vector<ReferenceValue*> local_reference_vars;
    for (auto var : value->local_reference_vars_) {
      local_reference_vars.push_back(heap.allocate<ReferenceValue>(Value::makeNone()));
    }
    for (auto var : references) {
      local_reference_vars.push_back(var);
    }
      
    if (has_option(OPTION_MACHINE_CODE_ONLY)) {
      if (!is_compiled) {
        InstructionList ir = IR::Compiler(heap, value).compile();
        compiled_func = ASM::Compiler(ir, *this).compile();
        is_compiled = true;
      }

      uint64_t result = compiled_func.call<uint64_t, void*, void*>(&arguments[0], &local_reference_vars[0]);
      return Value(result);
    } else {
      return interpreter->run_function(this, arguments, local_reference_vars);
    }
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
          std::cout << arguments[0].toString() << std::endl;
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
          StringValue* string = arguments[0].getPointer<StringValue>();
          try {
            return Value::makeInteger(std::stoi(string->toString()));
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
