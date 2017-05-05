#pragma once

namespace ASM {
  void helper_garbage_collect();
  uint64_t helper_alloc_record();
  uint64_t helper_read_global(uint64_t closure_p, int index);
  uint64_t helper_write_global(uint64_t closure_p, int index, uint64_t value);
  uint64_t helper_read_reference(uint64_t reference_p);
  void helper_write_reference(uint64_t reference_p, uint64_t value);
  uint64_t helper_read_function(uint64_t closure_p, int index);
  uint64_t helper_field_load(uint64_t closure, uint64_t record, int index);
  void helper_field_store(uint64_t closure, uint64_t record, int index, uint64_t value);
  uint64_t helper_index_load(uint64_t record, uint64_t index);
  void helper_index_store(uint64_t record, uint64_t index, uint64_t value);
  void helper_assert_int(uint64_t value);
  void helper_assert_not_zero(uint64_t value);
  void helper_assert_bool(uint64_t value);
  uint64_t helper_add(uint64_t left, uint64_t right);
  uint64_t helper_equals(uint64_t left, uint64_t right);
}