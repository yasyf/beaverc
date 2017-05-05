#include "../vm/globals.h"
#include "../vm/operations.h"
#include "../vm/Value.h"

using namespace VM;

namespace ASM {
  void helper_garbage_collect() {
    interpreter->potentially_garbage_collect();
  };

  uint64_t helper_alloc_record() {
    return Value::makePointer(interpreter->heap.allocate<RecordValue>()).value;
  }

  uint64_t helper_read_global(uint64_t closure_p, int index) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    std::string name = closure->value->names_[index];
    return interpreter->global_variables[name].value;
  }

  void helper_write_global(uint64_t closure_p, int index, uint64_t value) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    std::string name = closure->value->names_[index];
    interpreter->global_variables[name].value = value;
  }

  uint64_t helper_read_reference(uint64_t reference_p) {
    ReferenceValue* reference = (ReferenceValue*) reference_p;
    return reference->value.value;
  }

  void helper_write_reference(uint64_t reference_p, uint64_t value) {
    ((ReferenceValue*) reference_p)->value.value = value;
  }

  uint64_t helper_read_function(uint64_t closure_p, int index) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    auto function = closure->value->functions_[index];
    // allocate BareFunctionValue?
    #warning read function
    return 0;
  }

  uint64_t helper_field_load(uint64_t closure_p, uint64_t record_p, int index) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(closure->value->names_[index]).value;
  }

  void helper_field_store(uint64_t closure_p, uint64_t record_p, int index, uint64_t value) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    record->insert(closure->value->names_[index], Value(value));
  }

  uint64_t helper_index_load(uint64_t record_p, uint64_t index) {
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(Value(index).toString()).value;
  }

  void helper_index_store(uint64_t record_p, uint64_t index, uint64_t value) {
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    record->insert(Value(index).toString(), Value(value));
  }

  void helper_assert_int(uint64_t value) {
    Value(value).getInteger();
  }

  void helper_assert_not_zero(uint64_t value) {
    if (value == 0) {
      throw IllegalArithmeticException("divide by zero");
    }
  }

  void helper_assert_bool(uint64_t value) {
    Value(value).getBoolean();
  }

  uint64_t helper_add(uint64_t left, uint64_t right) {
    return add(Value(left), Value(right)).value;
  }

  uint64_t helper_equals(uint64_t left, uint64_t right) {
    return equals(Value(left), Value(right)).value;
  }
}
