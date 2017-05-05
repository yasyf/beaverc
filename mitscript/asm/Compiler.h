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

    void read_local(Var v, const R64& reg) {
      assm.mov(reg, local(v.num));
    }

    void read_temp(Temp t, const R64& reg) {
      assm.mov(reg, temp(t.num));
    }

    void write_local(Var v, const R64& reg) {
      assm.mov(local(v.num), reg);
    }

    void write_temp(Temp t, const M64& mem) {
      assm.mov(r10, mem);
      assm.mov(temp(t.num), r10);
    }

    void write_temp(Temp t, const R64& reg) {
      assm.mov(temp(t.num), reg);
    }

    void write_temp(Temp t, uint64_t cons) {
      assm.mov(r10, Imm64{cons});
      assm.mov(temp(t.num), r10);
    }

    void call_helper(void* fn, const R64& arg1) {
      assm.mov(rdi, arg1);
      assm.mov(r10, Imm64{(uint64_t)fn});
      assm.call(r10);
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

    void preamble() {
      void helper_setup_function(Value* arguments, ReferenceValue* refs, Value* base_pointer, uint64_t closure_p, ) 
      // preconditions:
      // rdi contains a pointer to the list of arguments
      // rsi contains a pointer to the list of references (local references and free vars)
      // Stuff that needs to happen:
      // Extend the stack RESERVED_STACK_SPACE + num_locals*8 + num_temps*8 downward
      // Store references list into the special place
      // Call the setup_function helper. It is called with:
        // The closure, the list of arguments, the list of references, the base pointer
      // It does the following:
        // Writes none to every variable
        // Takes the arguments and assigns them correctly on the stack
        // Adds the local variables (but not local reference variables) to the set of roots

      assm.sub(rsp, Imm64{(num_locals + num_temps + RESERVED_STACK_SPACE)*STACK_VALUE_SIZE});
      assm.mov(current_refs(), rsi);
      assm.mov(rdx, rbp);
      uint64_t addr = VM::Value::makePointer(&closure).value;
      assm.mov(rcx, Imm64{addr});
      assm.mov(r10, Imm64{(uint64_t)&helper_setup_function});
      assm.call(r10);
    }

    void compile(IR::InstructionList& ir) {
      assm.start(function);

      preamble();

      for (auto instruction : ir) {
        switch (instruction->op()) {
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              read_local(assign->src, r10);
              write_temp(assign->dest, r10);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              write_temp(assign->dest, assign->src.val);
            } else if (auto assign = dynamic_cast<Assign<RetVal>*>(instruction)) {
              write_temp(assign->dest, rax);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              read_temp(assign->src, r10);
              write_temp(assign->dest, r10);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              assm.mov(r10, current_refs());
              assm.mov(r10, M64{r10, (uint32_t)assign->src.num * STACK_VALUE_SIZE});
              write_temp(assign->dest, r10);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              assm.mov(r10, current_refs());
              assm.mov(r10, M64{r10, (uint32_t)assign->src.num * STACK_VALUE_SIZE});
              call_helper((void*) &helper_read_reference, r10);
              write_temp(assign->dest, rax);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              assm.mov(r11, Imm64{(uint32_t)assign->src.num});
              call_helper((void *)(&helper_read_global), r10, r11);
              write_temp(assign->dest, rax);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              assm.mov(r11, Imm64{(uint32_t)assign->src.num});
              call_helper((void *)(&helper_read_function), r10, r11);
              write_temp(assign->dest, rax);
            }
            break;
          }
          case IR::Operation::Store: {
            if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
              read_temp(store->src, r10);
              write_local(store->dest, r10);
            } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
              assm.mov(r10, current_refs());
              assm.mov(r10, M64{r10, (uint32_t)store->dest.num * STACK_VALUE_SIZE});
              read_temp(store->src, r11);
              assm.mov(M64{r10}, r11);
            } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              assm.mov(r11, Imm64{(uint32_t)store->dest.num});
              read_temp(store->src, rax);
              call_helper((void *)(&helper_write_global), r10, r11, rax);
            }
            break;
          }
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            read_temp(add->src1, r10);
            read_temp(add->src2, r11);
            call_helper((void *)(&helper_add), r10, r11);
            write_temp(add->dest, rax);
            break;
          }
          case IR::Operation::Sub: {
            auto sub = dynamic_cast<Sub*>(instruction);
            read_temp(sub->src1, r10);
            read_temp(sub->src2, r11);
            assm.sub(r10, r11);
            write_temp(sub->dest, r10);
            break;
          }
          case IR::Operation::Mul: {
            auto mul = dynamic_cast<Mul*>(instruction);
            read_temp(mul->src1, r10);
            read_temp(mul->src2, r11);
            assm.imul(r10, r11);
            assm.sar(r10, Imm8{3});
            write_temp(mul->dest, r10);
            break;
          }
          case IR::Operation::Div: {
            auto div = dynamic_cast<Div*>(instruction);
            read_temp(div->src1, r10);
            read_temp(div->src2, r11);
            assm.xor_(rdx, rdx);
            assm.mov(rax, r10);
            assm.idiv(r11);
            assm.sal(rax, Imm8{3});
            write_temp(div->dest, rax);
            break;
          }
          case IR::Operation::Neg: {
            auto neg = dynamic_cast<Neg*>(instruction);
            read_temp(neg->src, r10);
            assm.neg(r10);
            write_temp(neg->dest, r10);
            break;
          }
          case IR::Operation::Not: {
            auto nott = dynamic_cast<Not*>(instruction);
            read_temp(nott->src, r10);
            assm.xor_(r10, Imm16{0b1000});
            write_temp(nott->dest, r10);
            break;
          }
          case IR::Operation::Jump: {
            auto jump = dynamic_cast<Jump*>(instruction);
            assm.jmp(Rel32{jump->delta});
            break;
          }
          case IR::Operation::CondJump: {
            auto cjump = dynamic_cast<CondJump*>(instruction);
            uint64_t true_val = VM::Value::makeBoolean(true).value;
            read_temp(cjump->cond, r10);
            assm.cmp(r10, Imm32{(uint32_t)true_val});
            assm.je(Rel32{cjump->delta});
            break;
          }
        }
      }
      assm.finish();
    }

  public:
    Compiler(IR::InstructionList& ir, VM::ClosureFunctionValue& closure) : ir(ir), closure(closure) {
      this->num_locals = closure.value->local_vars_.size();
      IR::Return& return_instruction = dynamic_cast<IR::Return&>(ir.back());
      this->num_temps = return_instruction.val.num + 1;
    }

    x64asm::Function compile() {
      compile(ir);
      return function;
    }
  };
}
