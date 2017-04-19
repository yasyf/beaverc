#include "Value.h"
#include "Interpreter.h"

namespace VM {
  Value* BareFunctionValue::call(Interpreter & interpreter, std::vector<Value*> & arguments) {
    return interpreter.run_function(*value, arguments, std::vector<ReferenceValue*>());
  }

  Value* ClosureFunctionValue::call(Interpreter & interpreter, std::vector<Value*> & arguments) {
    return interpreter.run_function(*value, arguments, references);
  }

  Value* BuiltInFunctionValue::call(Interpreter & interpreter, std::vector<Value*> & arguments) {
    switch (type) {
        case BuiltInFunctionType::Print: {
          if (arguments.size() != 1) {
            throw RuntimeException("Wrong number of arguments to print");
          }
          #if DEBUG
          std::cout << "===== ";
          #endif
          std::cout << arguments[0]->toString() << std::endl;
          return heap.allocate<NoneValue>();
        }
        break;

        case BuiltInFunctionType::Input: {
          if (arguments.size() != 0) {
            throw RuntimeException("Wrong number of arguments to input");
          }
          std::string input;
          std::cin >> input;
          return heap.allocate<StringValue>(input);
        }
        break;

        case BuiltInFunctionType::Intcast: {
          if (arguments.size() != 1) {
            throw RuntimeException("Wrong number of arguments to intcast");
          }
          StringValue* string = force_cast<StringValue>(arguments[0]);
          try {
            return heap.allocate<IntegerValue>(std::stoi(string->toString()));
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
