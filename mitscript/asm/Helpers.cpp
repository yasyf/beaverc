#include "Helpers.h"
#include "../vm/globals.h"
#include "../vm/operations.h"

using namespace std;
using namespace VM;

namespace ASM {
  void helper_garbage_collect() {
    #if DEBUG
      cout << "helper_garbage_collect" << endl;
    #endif
    interpreter->potentially_garbage_collect();
  };

  uint64_t helper_alloc_record() {
    #if DEBUG
      cout << "helper_alloc_record" << endl;
    #endif
    return Value::makePointer(interpreter->heap.allocate<RecordValue>()).value;
  }

  uint64_t helper_call_function(uint64_t closure_p, Value* args, int argc) {
    #if DEBUG
      cout << "helper_call_function" << endl;
      cout << "Closure: " << (void*) closure_p << endl;
      cout << "Args: " << args << endl;
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
      cout << "helper_read_global" << endl;
      cout << "Closure: " << (void*) closure << endl;
      cout << "name: " << name << endl;
    #endif
    return interpreter->global_variables[name].value;
  }

  void helper_write_global(ClosureFunctionValue* closure, int index, uint64_t value) {
    std::string name = closure->value->names_[index];
    #if DEBUG
      cout << "helper_write_global" << endl;
      cout << "Closure: " << (void*) closure << endl;
      cout << "name: " << name << endl;
    #endif
    interpreter->global_variables[name].value = value;
  }

  uint64_t helper_read_reference(uint64_t reference_p) {
    #if DEBUG
      cout << "helper_read_reference" << endl;
    #endif
    ReferenceValue* reference = (ReferenceValue*) reference_p;
    return reference->value.value;
  }

  void helper_write_reference(uint64_t reference_p, uint64_t value) {
    #if DEBUG
      cout << "helper_write_reference" << endl;
    #endif
    ((ReferenceValue*) reference_p)->write(Value(value));
  }

  uint64_t helper_read_function(ClosureFunctionValue* closure, int index) {
    #if DEBUG
      cout << "helper_read_function" << endl;
    #endif
    return allocateFunction(closure, index).value;
  }

  uint64_t helper_field_load(ClosureFunctionValue* closure, uint64_t record_p, int index) {
    #if DEBUG
      cout << "helper_field_load" << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(closure->value->names_[index]).value;
  }

  void helper_field_store(ClosureFunctionValue* closure, uint64_t record_p, int index, uint64_t value) {
    #if DEBUG
      cout << "helper_field_store" << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    record->insert(closure->value->names_[index], Value(value));
  }

  uint64_t helper_index_load(uint64_t record_p, uint64_t index) {
    #if DEBUG
      cout << "helper_index_load" << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(Value(index).toString()).value;
  }

  void helper_index_store(uint64_t record_p, uint64_t index, uint64_t value) {
    #if DEBUG
      cout << "helper_index_store" << endl;
    #endif
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    record->insert(Value(index).toString(), Value(value));
  }

  void helper_assert_int(uint64_t value) {
    #if DEBUG
      cout << "helper_assert_int" << endl;
      cout << "Value: " << value << endl;
    #endif
    Value(value).getInteger();
  }

  void helper_assert_not_zero(uint64_t value) {
    #if DEBUG
      cout << "helper_assert_not_zero" << endl;
      cout << "Value: " << value << endl;
    #endif
    if (value == 0) {
      throw IllegalArithmeticException("divide by zero");
    }
  }

  void helper_assert_bool(uint64_t value) {
    #if DEBUG
      cout << "helper_assert_bool" << endl;
      cout << "Value: " << value << endl;
    #endif
    Value(value).getBoolean();
  }

  uint64_t helper_add(uint64_t left, uint64_t right) {
    #if DEBUG
      cout << "helper_add" << endl;
    #endif
    return add(Value(left), Value(right)).value;
  }

  uint64_t helper_equals(uint64_t left, uint64_t right) {
    #if DEBUG
      cout << "helper_equals" << endl;
    #endif
    return equals(Value(left), Value(right)).value;
  }

  uint64_t helper_convert_to_closure(uint64_t bare_function) {
    #if DEBUG
      cout << "helper_convert_to_closure" << endl;
    #endif
    BareFunctionValue* func = Value(bare_function).getPointer<BareFunctionValue>();
    return Value::makePointer(interpreter->heap.allocate<ClosureFunctionValue>(func->value)).value;
  }

  uint64_t helper_add_reference_to_closure(uint64_t closure, uint64_t reference) {
    #if DEBUG
      cout << "helper_add_reference_to_closure" << endl;
    #endif
    ClosureFunctionValue* c = Value(closure).getPointer<ClosureFunctionValue>();
    c->add_reference((ReferenceValue*) reference);
    return closure;
  }
}
