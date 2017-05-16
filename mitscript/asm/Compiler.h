#pragma once
#include "include/x64asm.h"
#include "../Debug.h"
#include "../ir/Instructions.h"
#include "../vm/Value.h"
#include "Helpers.h"
#include "Exception.h"
#include <experimental/optional>
#include <set>
#include <map>
#include <cassert>

using namespace std;
using namespace x64asm;
using namespace IR;

#define RESERVED_STACK_SPACE 4
#define STACK_VALUE_SIZE 8

#define IR_INSTRUCTION_BYTE_UPPER_BOUND 64

namespace ASM {
  static constexpr std::array<R64, 8> caller_saved_regs = {
    rcx, rdx, rsi, rdi, r8,  r9,  r10, r11
  };

  static constexpr std::array<R64, 4> arg_regs = {
    rdi, rsi, rdx, rcx
  };

  class Compiler {
    IR::InstructionList& ir;
    Assembler assm;
    size_t num_temps;
    set<R64> live;
    map<R64, shared_ptr<Temp>> reg_temps;

    M64 current_closure();
    M64 current_locals();
    M64 current_refs();
    void reg_move(const R64& dest, const R64& src);
    void alive(const R64& reg);
    void alive(shared_ptr<Temp> temp);
    bool is_alive(const R64& reg);
    void dead(const R64& reg);
    void dead(shared_ptr<Temp> temp);
    void reserve(const R64& reg);
    R64 alloc_reg();
    M64 temp_mem(size_t i);
    void assign_mem_to_temp(shared_ptr<Temp> dest, M64 base, int num);
    void store_temp_to_mem(shared_ptr<Temp> src, M64 base, int num);
    void assign_helper_call_to_temp(shared_ptr<Temp> dest, void* helper, int num);
    void assign_local(shared_ptr<Var> src, shared_ptr<Temp> dest);
    void store_local(shared_ptr<Temp> src, shared_ptr<Var> dest);
    void assign_ref(shared_ptr<Ref> src, shared_ptr<Temp> dest);
    void assign_deref(shared_ptr<Deref> src, shared_ptr<Temp> dest);
    void store_deref(shared_ptr<Temp> src, shared_ptr<Deref> dest);
    void assign_glob(shared_ptr<Glob> src, shared_ptr<Temp> dest);
    void store_glob(shared_ptr<Temp> src, shared_ptr<Glob> dest);
    void assign_function(shared_ptr<IR::Function> src, shared_ptr<Temp> dest);
    void extract_bits(shared_ptr<Temp> temp, const R64& dest, size_t start, size_t length);
    R64 read_temp(shared_ptr<Temp> temp, const R64& reg_hint);
    R64 read_temp(shared_ptr<Temp> temp);
    void write_temp(shared_ptr<Temp> temp, const R64& reg);
    void write_temp(shared_ptr<Temp> temp, uint64_t cons);
    void prepare_call_helper(size_t argc);
    void call_helper(void* fn, const R64 args[], size_t argc);
    void call_helper(void* fn);
    void call_helper(void* fn, const R64 arg1);
    void call_helper(void* fn, const R64 arg1, const R64 arg2);
    void call_helper(void* fn, const R64 arg1, const R64 arg2, const R64 arg3);
    void call_helper(void* fn, const R64 arg1, const R64 arg2, const R64 arg3, const R64 arg4);
    void preamble();
    void postamble(const R64& retval);
    void compile(IR::InstructionList& ir, x64asm::Function& function);

  public:
    Compiler(IR::InstructionList& ir, size_t num_temps);
    void compileInto(x64asm::Function& func);
  };
}
