#include "Helpers.h"
#include "../vm/globals.h"
#include "../vm/operations.h"

using namespace std;
using namespace VM;

namespace ASM {
  void helper_garbage_collect() {
    #if DEBUG
      cout << endl << "helper_garbage_collect" << endl;
    #endif
    interpreter->potentially_garbage_collect();
  };

  uint64_t helper_alloc_record() {
    #if DEBUG
      cout << endl << "helper_alloc_record" << endl;
    #endif
    return Value::makePointer(interpreter->heap.allocate<RecordValue>()).value;
  }

  uint64_t helper_call_function(uint64_t closure_p, Value* args, int argc) {
    #if DEBUG
      cout << endl << "helper_call_function" << endl;
      cout << "Closure: " << (void*) closure_p << endl;
      cout << "Args: [";
      for (size_t i = 0; i < argc; ++i) {
        cout << args[i].toString() << ", ";
      }
      cout << "]" << endl;
      cout << "Index: " << argc << endl;
    #endif
    AbstractFunctionValue* closure = Value(closure_p).getPointer<AbstractFunctionValue>();
    std::vector<Value> values;
    for (size_t i = 0; i < argc; ++i) {
      values.push_back(args[i]);
    }
    return closure->call(values).value;
  }

  uint64_t helper_read_global(ClosureFunctionValue* closure, int index) {
    std::string name = closure->value->names_[index];
    #if DEBUG
      cout << endl << "helper_read_global" << endl;
      cout << "Closure: " << (void*) closure << endl;
      cout << "name: " << name << endl;
      cout << "value: " << Value(interpreter->global_variables[name].value).toString() << endl;
    #endif
    return interpreter->global_variables[name].value;
  }

  void helper_write_global(ClosureFunctionValue* closure, int index, uint64_t value) {
    std::string name = closure->value->names_[index];
    #if DEBUG
      cout << endl << "helper_write_global" << endl;
      cout << "Closure: " << (void*) closure << endl;
      cout << "name: " << name << endl;
      cout << "value: " << Value(value).toString() << endl;
    #endif
    interpreter->global_variables[name].value = value;
  }

  uint64_t helper_read_reference(uint64_t reference_p) {
    #if DEBUG
      cout << endl << "helper_read_reference" << endl;
    #endif
    ReferenceValue* reference = (ReferenceValue*) reference_p;
    return reference->value.value;
  }

  void helper_write_reference(uint64_t reference_p, uint64_t value) {
    #if DEBUG
      cout << endl << "helper_write_reference" << endl;
    #endif
    ((ReferenceValue*) reference_p)->write(Value(value));
  }

  uint64_t helper_read_function(ClosureFunctionValue* closure, int index) {
    #if DEBUG
      cout << endl << "helper_read_function" << endl;
    #endif
    return allocateFunction(closure, index).value;
  }

  uint64_t helper_field_load(ClosureFunctionValue* closure, uint64_t record_p, int index) {
    #if DEBUG
      cout << endl << "helper_field_load" << " Name: " << closure->value->names_[index] << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(closure->value->names_[index]).value;
  }

  void helper_field_store(ClosureFunctionValue* closure, uint64_t record_p, int index, uint64_t value) {
    #if DEBUG
      cout << endl << "helper_field_store" << " Name: " << closure->value->names_[index] << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    record->insert(closure->value->names_[index], Value(value));
  }

  uint64_t helper_index_load(uint64_t record_p, uint64_t index) {
    #if DEBUG
      cout << endl << "helper_index_load" << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(Value(index).toString()).value;
  }

  void helper_index_store(uint64_t record_p, uint64_t index, uint64_t value) {
    #if DEBUG
      cout << endl << "helper_index_store" << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    record->insert(Value(index).toString(), Value(value));
  }

  void helper_throw_not_int() {
    throw IllegalCastException("Value is not a integer");
  }

  void helper_throw_zero() {
    throw IllegalArithmeticException("divide by zero");
  }

  void helper_throw_not_bool() {
    throw IllegalCastException("Value is not a boolean");
  }

  void helper_throw_uninitialized(ClosureFunctionValue* closure, int index) {
    std::string name = closure->value->names_[index];
    throw UninitializedVariableException(name);
  }

  uint64_t helper_add(uint64_t left, uint64_t right) {
    #if DEBUG
      cout << endl << "helper_add" << endl;
      cout << Value(left).toString()
           << " + "
           << Value(right).toString();
      cout << " = "
           << add(Value(left), Value(right)).toString()
           << endl;
    #endif
    return add(Value(left), Value(right)).value;
  }

  uint64_t helper_equals(uint64_t left, uint64_t right) {
    #if DEBUG
      cout << endl << "helper_equals" << endl;
    #endif
    return equals(Value(left), Value(right)).value;
  }

  uint64_t helper_convert_to_closure(uint64_t bare_function) {
    #if DEBUG
      cout << endl << "helper_convert_to_closure" << endl;
    #endif
    BareFunctionValue* func = Value(bare_function).getPointer<BareFunctionValue>();
    return Value::makePointer(interpreter->heap.allocate<ClosureFunctionValue>(func->value)).value;
  }

  uint64_t helper_add_reference_to_closure(uint64_t closure, uint64_t reference) {
    #if DEBUG
      cout << endl << "helper_add_reference_to_closure" << endl;
    #endif
    ClosureFunctionValue* c = Value(closure).getPointer<ClosureFunctionValue>();
    c->add_reference((ReferenceValue*) reference);
    return closure;
  }
}
