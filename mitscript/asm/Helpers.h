#pragma once
#include "../vm/Value.h"

namespace ASM {
  void helper_garbage_collect();
  uint64_t helper_alloc_record();
  uint64_t helper_read_reference(uint64_t reference_p);
  void helper_write_reference(uint64_t reference_p, uint64_t value);
  uint64_t helper_read_global(VM::ClosureFunctionValue* closure, int index);
  void helper_write_global(VM::ClosureFunctionValue* closure, int index, uint64_t value);
  uint64_t helper_read_function(VM::ClosureFunctionValue* closure, int index);
  uint64_t helper_field_load(VM::ClosureFunctionValue* closure, uint64_t record_p, int index);
  void helper_field_store(VM::ClosureFunctionValue* closure, uint64_t record_p, int index, uint64_t value);
  uint64_t helper_index_load(uint64_t record, uint64_t index);
  void helper_index_store(uint64_t record, uint64_t index, uint64_t value);
  void helper_assert_int(uint64_t value);
  void helper_assert_not_zero(uint64_t value);
  void helper_assert_bool(uint64_t value);
  uint64_t helper_add(uint64_t left, uint64_t right);
  uint64_t helper_equals(uint64_t left, uint64_t right);
  uint64_t helper_call_function(uint64_t closure_p, VM::Value* args, int argc);
  uint64_t helper_convert_to_closure(uint64_t bare_function);
  void helper_add_reference_to_closure(uint64_t closure, uint64_t reference);
}