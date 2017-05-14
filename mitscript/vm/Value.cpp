#include "Value.h"
#include "Interpreter.h"
#include "globals.h"
#include "../ir/OptimizingCompiler.h"
#include "../asm/Compiler.h"
#include <list>

namespace VM {

  std::string Value::toString() const {
    switch (value & _VALUE_MASK) {
      case _INTEGER_TAG: {
        return std::to_string(getInteger());
      }
      case _NONE_TAG: {
        return "None";
      }
      case _BOOLEAN_TAG: {
        return (getBoolean()) ? "True" : "False";
      }
      case _STRING_CONSTANT_TAG: {
        return std::string(getStringConstant());
      }
      case _STRING_VALUE_TAG:
      case _POINTER_TAG: {
        return getPointerValue()->toString();
      }
    }
  }

  StringValue::StringValue(GC::CollectedHeap& heap, const std::string& value) : PointerValue(heap) {
    height = 0;
    length = value.size();
    if (length > 0) {
      memory = static_cast<char*>(malloc(length * sizeof(char)));
      strncpy(memory, value.c_str(), length);
    }
    heap.increaseSize(size());
  }

  StringValue::StringValue(GC::CollectedHeap& heap, const Value l, const Value r) : PointerValue(heap) {
    if (l.isPointer() && !l.isStringValue()) {
      // Must be a record or function
      left = Value::makeString(heap.allocate<StringValue>(l.toString()));
    } else {
      left = l;
    }
    if (r.isPointer() && !r.isStringValue()) {
      // Must be a record or function
      right = Value::makeString(heap.allocate<StringValue>(r.toString()));
    } else {
      right = r;
    }
    height = 1;
    if (left.isPointer()) {
      // Must be a string now
      StringValue* ll = left.getPointer<StringValue>();
      height = max(height, ll->height + 1);
    }
    if (right.isPointer()) {
      StringValue* rr = right.getPointer<StringValue>();
      height = max(height, rr->height + 1);
    }
  }

  StringValue::~StringValue() {
    #ifdef DEBUG
    cout << "DELETING StringValue: " << toString() << endl;
    #endif
    if (height == 0 && length > 0) {
      free(memory);
    }
    heap.decreaseSize(size());
  }

  std::string StringValue::toString() {
    if (height == 0) {
      return std::string(memory, length);
    }
    return left.toString() + right.toString();
  };

  size_t StringValue::size() {
    if (height == 0) {
      return sizeof(StringValue) + length * sizeof(char);
    } else {
      return sizeof(StringValue);
    }
  }

  void StringValue::markChildren(size_t generation, bool mark_recent_only) {
    if (height == 0) return;
    if (left.isPointer()) {
      left.getPointerValue()->mark(generation, mark_recent_only);
    }
    if (right.isPointer()) {
      right.getPointerValue()->mark(generation, mark_recent_only);
    }
  }

  RecordValue::RecordValue(GC::CollectedHeap& heap) : PointerValue(heap) {
    heap.increaseSize(size());
  }

  RecordValue::~RecordValue() {
    #ifdef DEBUG
    cout << "DELETING RecordValue: " << toString() << endl;
    #endif
    heap.decreaseSize(size());
  }

  Value RecordValue::get(std::string key) {
    return values[key];
  }

  void RecordValue::insert(std::string key, Value inserted) {
    if (values.count(key) == 0)
      heap.increaseSize(sizeof(std::string) + key.capacity() * sizeof(char) + sizeof(Value));
    if (has_optimization(OPTIMIZATION_GC_GENERATIONAL) &&
        inserted.isPointer() &&
        inserted.getPointerValue()->generation == GC::Generation::RecentlyAllocated &&
        this->generation != GC::Generation::RecentlyAllocated) {
      heap.cross_generation_pointers.push_back(this);
    }
    values[key] = inserted;
  }

  std::string RecordValue::toString() {
    std::string result = "{";
    for (auto keyvalue : values) {
        result += keyvalue.first + ":" + keyvalue.second.toString() + " ";
    };
    result += "}";
    return result;
  }

  size_t RecordValue::size() {
    size_t s = sizeof(RecordValue);
    for (auto pair : values) {
      s += sizeof(std::string) + pair.first.capacity() * sizeof(char) + sizeof(Value);
    }
    return s;
  }

  void RecordValue::markChildren(size_t generation, bool mark_recent_only) {
    for (auto& pair : values) {
      if (pair.second.isPointer()) {
        pair.second.getPointerValue()->mark(generation, mark_recent_only);
      }
    }
  }

  ReferenceValue::ReferenceValue(GC::CollectedHeap& heap, Value v) : PointerValue(heap), value(v) {
    heap.increaseSize(size());
  }

  ReferenceValue::~ReferenceValue() {
    #ifdef DEBUG
    cout << "DELETING ReferenceValue: " << toString() << endl;
    #endif
    heap.decreaseSize(size());
  }

  std::string ReferenceValue::toString() {
    #if DEBUG
      return "ref";
    #else
      throw RuntimeException("You have uncovered a bug :(");
    #endif
  }

  void ReferenceValue::write(Value v) {
    if (has_optimization(OPTIMIZATION_GC_GENERATIONAL) &&
        v.isPointer() &&
        v.getPointerValue()->generation == GC::Generation::RecentlyAllocated &&
        this->generation != GC::Generation::RecentlyAllocated) {
      heap.cross_generation_pointers.push_back(this);
    }
    value = v;
  }

  size_t ReferenceValue::size() {
    return sizeof(ReferenceValue);
  }

  void ReferenceValue::markChildren(size_t generation, bool mark_recent_only) {
    if (value.isPointer()) {
      value.getPointerValue()->mark(generation, mark_recent_only);
    }
  }

  std::string AbstractFunctionValue::toString() {
    return "FUNCTION";
  }

  BareFunctionValue::BareFunctionValue(GC::CollectedHeap& heap, std::shared_ptr<BC::Function> value) : AbstractFunctionValue(heap), value(value) {
      heap.increaseSize(size());
    }

  BareFunctionValue::~BareFunctionValue() {
    #ifdef DEBUG
    cout << "DELETING BareFunctionValue: " << toString() << endl;
    #endif
    heap.decreaseSize(size());
  }

  Value BareFunctionValue::call(std::vector<Value> & arguments) {
    throw RuntimeException("call on a BareFunctionValue");
  }

  ClosureFunctionValue::ClosureFunctionValue(GC::CollectedHeap& heap, std::shared_ptr<BC::Function> value) : AbstractFunctionValue(heap), value(value) {
    heap.increaseSize(size());
  }

  ClosureFunctionValue::~ClosureFunctionValue() {
    #ifdef DEBUG
    cout << "DELETING ClosureFunctionValue: " << toString() << endl;
    #endif
    heap.decreaseSize(size());
  }


  void ClosureFunctionValue::add_reference(ReferenceValue* reference) {
    heap.increaseSize(sizeof(ReferenceValue*));
    references.push_back(reference);
  };

  size_t ClosureFunctionValue::size() {
    return sizeof(ClosureFunctionValue) + references.size() * sizeof(ReferenceValue*);
  }

  void ClosureFunctionValue::markChildren(size_t generation, bool mark_recent_only) {
    for (auto ref : references)
      ref->mark(generation, mark_recent_only);
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
            local_reference_vars[reverse_index[var_name]]->write(arguments[i]);
        }
      }
    }

    if (has_optimization(OPTIMIZATION_MACHINE_CODE) && !value->is_compiled) {
      InstructionList ir;
      IR::OptimizingCompiler ir_compiler(value, ir);
      size_t temp_count = ir_compiler.compile(has_optimization(OPTIMIZATION_OPTIMIZATION_PASSES));
      ASM::Compiler asm_compiler(ir, temp_count);
      asm_compiler.compileInto(value->compiled_function);
      value->is_compiled = !has_optimization(OPTIMIZATION_COMPILE_ONLY);
    }

    interpreter->push_frame(&local_vars, &local_reference_vars);

    Value result;
    if (value->is_compiled) {
      result = Value(value->compiled_function.call<uint64_t, void*, void*, void*>(this, &local_vars[0], &local_reference_vars[0]));
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


  BuiltInFunctionValue::BuiltInFunctionValue(GC::CollectedHeap& heap, int t) : AbstractFunctionValue(heap) {
    heap.increaseSize(size());
    type = static_cast<BuiltInFunctionType>(t);
  }

  BuiltInFunctionValue::~BuiltInFunctionValue() {
    #ifdef DEBUG
    cout << "DELETING BuiltinFunctionValue: " << toString() << endl;
    #endif
    heap.decreaseSize(size());
  }
}
