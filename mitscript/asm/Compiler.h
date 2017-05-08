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
    size_t num_temps;

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

    void call_helper(void* fn) {
      assm.mov(r10, Imm64{(uint64_t)fn});
      assm.call(r10);
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

    void call_helper(void* fn, const R64& arg1, const R64& arg2, const R64& arg3, const R64& arg4) {
      assm.mov(rdi, arg1);
      assm.mov(rsi, arg2);
      assm.mov(rdx, arg3);
      assm.mov(rcx, arg4);
      assm.mov(r10, Imm64{(uint64_t)fn});
      assm.call(r10);
    }

    void preamble() {
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

      assm.push(rbp);
      assm.mov(rbp, rsp);

      assm.push(rbx);
      assm.push(r12);
      assm.push(r13);
      assm.push(r14);
      assm.push(r15);

      assm.sub(rsp, Imm32{((uint32_t)num_locals + (uint32_t)num_temps + RESERVED_STACK_SPACE)*STACK_VALUE_SIZE});
      assm.mov(current_refs(), rsi);
      assm.mov(rdx, rbp);
      uint64_t addr = VM::Value::makePointer(&closure).value;
      assm.mov(rcx, Imm64{addr});
      assm.mov(r10, Imm64{(uint64_t)&helper_setup_function});
      assm.call(r10);
    }

    void postamble(const R64& retval) {
      assm.add(rsp, Imm32{((uint32_t)num_locals + (uint32_t)num_temps + RESERVED_STACK_SPACE)*STACK_VALUE_SIZE});
      #warning clean up garbage collection here but not yet.
      assm.mov(rax, retval);
      assm.pop(r15);
      assm.pop(r14);
      assm.pop(r13);
      assm.pop(r12);
      assm.pop(rbx);
      assm.pop(rbp);
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
            assm.sub(r11, r10);
            write_temp(sub->dest, r11);
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
          case IR::Operation::Gt: {
            auto gt = dynamic_cast<Gt*>(instruction);
            read_temp(gt->src1, r10);
            read_temp(gt->src2, r11);
            assm.xor_(rax, rax);
            assm.cmp(r11, r10);
            assm.setg(rax);
            assm.sal(rax, Imm8{3});
            assm.or_(rax, Imm16{_BOOLEAN_TAG});
            write_temp(gt->dest, rax);
            break;
          }
          case IR::Operation::Geq: {
            auto gte = dynamic_cast<Geq*>(instruction);
            read_temp(gte->src1, r10);
            read_temp(gte->src2, r11);
            assm.xor_(rax, rax);
            assm.cmp(r11, r10);
            assm.setge(rax);
            assm.sal(rax, Imm8{3});
            assm.or_(rax, Imm16{_BOOLEAN_TAG});
            write_temp(gte->dest, rax);
            break;
          }
          case IR::Operation::Eq: {
            auto eq = dynamic_cast<Eq*>(instruction);
            read_temp(eq->src1, r10);
            read_temp(eq->src2, r11);
            call_helper((void *)(&helper_equals), r10, r11);
            write_temp(eq->dest, rax);
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
          case IR::Operation::Call: {
            auto call = dynamic_cast<IR::Call*>(instruction);
            for (int i = call->args.size() - 1; i >= 0; --i) {
              read_temp(call->args[i], r10);
              assm.push(r10);
            }
            assm.mov(r10, Imm32{(uint32_t)call->args.size()});
            read_temp(call->closure, r11);
            call_helper((void *)(&helper_call_function), r11, rsp, r10);
            assm.add(rsp, Imm32{call->args.size()*STACK_VALUE_SIZE});
            break;
          }
          case IR::Operation::Return: {
            auto ret = dynamic_cast<IR::Return*>(instruction);
            read_temp(ret->val, rbx);
            postamble(rbx);
            assm.ret();
            break;
          }
          case IR::Operation::CallHelper: {
            if (auto op = dynamic_cast<CallHelper<Helper::GarbageCollect>*>(instruction)) {
              call_helper((void *)(&helper_garbage_collect));
            } else if (auto op = dynamic_cast<CallHelper<Helper::AllocRecord>*>(instruction)) {
              call_helper((void *)(&helper_alloc_record));
            } else if (auto op = dynamic_cast<CallHelper<Helper::FieldLoad>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              read_temp(op->args[0], r11);
              assm.mov(r12, Imm64{op->arg0});
              call_helper((void *)(&helper_field_load), r10, r11, r12);
            } else if (auto op = dynamic_cast<CallHelper<Helper::FieldStore>*>(instruction)) {
              uint64_t addr = VM::Value::makePointer(&closure).value;
              assm.mov(r10, Imm64{addr});
              read_temp(op->args[0], r11);
              assm.mov(r12, Imm64{op->arg0});
              read_temp(op->args[1], r13);
              call_helper((void *)(&helper_field_load), r10, r11, r12, r13);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexLoad>*>(instruction)) {
              read_temp(op->args[0], r10);
              read_temp(op->args[1], r11);
              call_helper((void *)(&helper_index_load), r10, r11);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexStore>*>(instruction)) {
              read_temp(op->args[0], r10);
              read_temp(op->args[1], r11);
              read_temp(op->args[2], r12);
              call_helper((void *)(&helper_index_store), r10, r11, r12);
            } else if (auto op = dynamic_cast<CallHelper<Helper::AssertInt>*>(instruction)) {
              read_temp(op->args[0], r10);
              call_helper((void *)(&helper_assert_int), r10);
            } else if (auto op = dynamic_cast<CallHelper<Helper::AssertNotZero>*>(instruction)) {
              read_temp(op->args[0], r10);
              call_helper((void *)(&helper_assert_not_zero), r10);
            } else if (auto op = dynamic_cast<CallHelper<Helper::AssertBool>*>(instruction)) {
              read_temp(op->args[0], r10);
              call_helper((void *)(&helper_assert_bool), r10);
            }
          }
          case IR::Operation::AllocClosure: {
            auto op = dynamic_cast<AllocClosure*>(instruction);
            read_temp(op->function, r12);
            call_helper((void*) &helper_convert_to_closure, r12);
            assm.mov(r12, rax);
            for (Temp t : op->refs) {
              read_temp(t, r10);
              call_helper((void*) &helper_add_reference_to_closure, r12, r10);
            }
            write_temp(op->dest, r12);
            break;
          }
          case IR::Operation::And: {
            auto andd = dynamic_cast<And*>(instruction);
            read_temp(andd->src1, r10);
            read_temp(andd->src2, r11);
            assm.and_(r10, r11);
            write_temp(andd->dest, r10);
            break;
          }
          case IR::Operation::Or: {
            auto orr = dynamic_cast<Or*>(instruction);
            read_temp(orr->src1, r10);
            read_temp(orr->src2, r11);
            assm.or_(r10, r11);
            write_temp(orr->dest, r10);
            break;
          }
          default: {
            throw VM::RuntimeException("Oops we forgot to implement something\n");
          }
        }
      }
      assm.finish();
    }

  public:
    Compiler(IR::InstructionList& ir, VM::ClosureFunctionValue& closure) : ir(ir), closure(closure) {
      this->num_locals = closure.value->local_vars_.size();
      IR::Return* return_instruction = dynamic_cast<IR::Return*>(ir.back());
      this->num_temps = return_instruction->val.num + 1;
    }

    x64asm::Function compile() {
      compile(ir);
      return function;
    }
  };
}
