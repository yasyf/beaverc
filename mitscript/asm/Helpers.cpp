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

  uint64_t helper_read_global(uint64_t closure_p, int index) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    std::string name = closure->value->names_[index];
    #if DEBUG
      cout << "helper_read_global" << endl;
      cout << "Closure: " << (void*) closure_p << endl;
      cout << "name: " << name << endl;
    #endif
    return interpreter->global_variables[name].value;
  }

  void helper_write_global(uint64_t closure_p, int index, uint64_t value) {
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    std::string name = closure->value->names_[index];
    #if DEBUG
      cout << "helper_read_global" << endl;
      cout << "Closure: " << (void*) closure_p << endl;
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
    ((ReferenceValue*) reference_p)->value.value = value;
  }

  uint64_t helper_read_function(uint64_t closure_p, int index) {
    #if DEBUG
      cout << "helper_read_function" << endl;
    #endif
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    return allocateFunction(closure, index).value;
  }

  uint64_t helper_field_load(uint64_t closure_p, uint64_t record_p, int index) {
    #if DEBUG
      cout << "helper_field_load" << endl;
    #endif
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
    RecordValue* record = Value(record_p).getPointer<RecordValue>();
    return record->get(closure->value->names_[index]).value;
  }

  void helper_field_store(uint64_t closure_p, uint64_t record_p, int index, uint64_t value) {
    #if DEBUG
      cout << "helper_field_store" << endl;
    #endif
    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();
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

  void helper_setup_function(Value* arguments, ReferenceValue** refs, Value* base_pointer, uint64_t closure_p) {
    #if DEBUG
      cout << "helper_setup_function" << endl;
    #endif

    ClosureFunctionValue* closure = Value(closure_p).getPointer<ClosureFunctionValue>();

    #warning Make sure to add locals and temps to the set of roots

    // Assign everything to none
    for (int i = 0; i < closure->value->local_vars_.size(); i++) {
      base_pointer[-i-1] = Value::makeNone();
    }

    // Assign the variables
    std::map<std::string, int> reverse_index;
    for (int i = 0; i < closure->value->local_reference_vars_.size(); i++) {
      reverse_index[closure->value->local_reference_vars_[i]] = i;
    }

    for (int i = 0; i < closure->value->parameter_count_; i++) {
      std::string var_name = closure->value->local_vars_[i];
      #if DEBUG
      std::cout << "Assembly: ";
      std::cout << var_name << " = " << arguments[i].toString() << std::endl;
      #endif
      if (reverse_index.count(var_name) == 0) {
        base_pointer[-i-1] = arguments[i];
      } else {
        refs[reverse_index[var_name]]->value = arguments[i];
      }
    }
  }

  uint64_t helper_convert_to_closure(uint64_t bare_function) {
    BareFunctionValue* func = Value(bare_function).getPointer<BareFunctionValue>();
    return Value::makePointer(interpreter->heap.allocate<ClosureFunctionValue>(func->value)).value;
  }

  void helper_add_reference_to_closure(uint64_t closure, uint64_t reference) {
    ClosureFunctionValue* c = Value(closure).getPointer<ClosureFunctionValue>();
    c->add_reference(Value(reference).getPointer<ReferenceValue>());
  }
}
