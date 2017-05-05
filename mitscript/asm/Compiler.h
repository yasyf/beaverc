#pragma once
#include "include/x64asm.h"
#include "../ir/Instructions.h"
#include "../vm/Value.h"
#include "Helpers.h"

using namespace std;
using namespace x64asm;
using namespace IR;

#define RESERVED_STACK_SPACE 2
#define STACK_VALUE_SIZE 8

namespace ASM {
  class Compiler {
    IR::InstructionList& ir;
    VM::ClosureFunctionValue& closure;
    x64asm::Function function;
    Assembler assm;
    size_t num_locals;

    M64 current_refs() {
      #warning fill this in
      return M64{rbp, Imm32{(uint32_t)(-1 * STACK_VALUE_SIZE)}};
    }

    M64 local(size_t i) {
      return M64{rbp, Imm32{(uint32_t)(-(i+RESERVED_STACK_SPACE) * STACK_VALUE_SIZE)}};
    }

    M64 temp(size_t i) {
      return M64{rbp, Imm32{(uint32_t)(-(i+RESERVED_STACK_SPACE+num_locals) * STACK_VALUE_SIZE)}};
    }

    void read_local(size_t i, const R64& reg) {
      assm.mov(reg, local(i));
    }

    void read_temp(size_t i, const R64& reg) {
      assm.mov(reg, temp(i));
    }

    void write_local(size_t i, const R64& reg) {
      assm.mov(local(i), reg);
    }

    void write_temp(size_t i, const M64& mem) {
      assm.mov(r10, mem);
      assm.mov(temp(i), r10);
    }

    void write_temp(size_t i, const R64& reg) {
      assm.mov(temp(i), reg);
    }

    void write_temp(size_t i, uint64_t cons) {
      assm.mov(r10, Imm64{cons});
      assm.mov(temp(i), r10);
    }

    void call_helper(void* fn, const R64& arg1, const R64& arg2) {
      assm.mov(rdi, arg1);
      assm.mov(rsi, arg2);
      assm.mov(r10, Imm64{(uint64_t)fn});
      assm.call(r10);
    }

    void call_helper(void* fn, const R64& arg1, const R64& arg2, const R64& arg3) {
      assm.mov(rdi, arg1);
      assm.mov(rsi, arg2);
      assm.mov(rdx, arg3);
      assm.mov(r10, Imm64{(uint64_t)fn});
      assm.call(r10);
    }

    void compile(IR::InstructionList& ir) {
      assm.start(function);
      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              read_local(assign->src.num, r10);
              write_temp(assign->dest.num, r10);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              write_temp(assign->dest.num, assign->src.val);
            } else if (auto assign = dynamic_cast<Assign<RetVal>*>(instruction)) {
              write_temp(assign->dest.num, rax);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              read_temp(assign->src.num, r10);
              write_temp(assign->dest.num, r10);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              assm.mov(r10, current_refs());
              assm.mov(r10, M64{r10, (uint32_t)assign->src.num * STACK_VALUE_SIZE});
              write_temp(assign->dest.num, r10);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              assm.mov(r10, current_refs());
              assm.mov(r10, M64{r10, (uint32_t)assign->src.num * STACK_VALUE_SIZE});
              write_temp(assign->dest.num, M64{r10});
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              assm.mov(r11, Imm64{(uint32_t)assign->src.num});
              call_helper((void *)(&helper_read_global), r10, r11);
              write_temp(assign->dest.num, rax);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              assm.mov(r11, Imm64{(uint32_t)assign->src.num});
              call_helper((void *)(&helper_read_function), r10, r11);
              write_temp(assign->dest.num, rax);
            }
            break;
          }
          case IR::Operation::Store: {
            if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
              read_temp(store->src.num, r10);
              write_local(store->dest.num, r10);
            } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
              assm.mov(r10, current_refs());
              assm.mov(r10, M64{r10, (uint32_t)store->dest.num * STACK_VALUE_SIZE});
              read_temp(store->src.num, r11);
              assm.mov(M64{r10}, r11);
            } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              assm.mov(r11, Imm64{(uint32_t)store->dest.num});
              read_temp(store->src.num, rax);
              call_helper((void *)(&helper_write_global), r10, r11, rax);
            }
            break;
          }
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            read_temp(add->src1.num, r10);
            read_temp(add->src2.num, r11);
            call_helper((void *)(&helper_add), r10, r11);
            write_temp(add->dest.num, rax);
            break;
          }
          case IR::Operation::Sub: {
            auto sub = dynamic_cast<Sub*>(instruction);
            read_temp(sub->src1.num, r10);
            read_temp(sub->src2.num, r11);
            assm.sub(r10, r11);
            write_temp(sub->dest.num, r10);
          }
          case IR::Operation::Mul: {
            auto mul = dynamic_cast<Mul*>(instruction);
            read_temp(mul->src1.num, r10);
            read_temp(mul->src2.num, r11);
            assm.imul(r10, r11);
            write_temp(mul->dest.num, r10);
          }
        }
      }
      assm.finish();
    }

  public:
    Compiler(IR::InstructionList& ir, VM::ClosureFunctionValue& closure) : ir(ir), closure(closure) {
      this->num_locals = closure.value->local_vars_.size();
    }

    x64asm::Function compile() {
      compile(ir);
      return function;
    }
  };
}
