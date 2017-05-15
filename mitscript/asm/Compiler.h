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

    M64 current_closure() {
      return M64{rbp, Imm32{(uint32_t)(-1 * STACK_VALUE_SIZE)}};
    }

    M64 current_locals() {
      return M64{rbp, Imm32{(uint32_t)(-2 * STACK_VALUE_SIZE)}};
    }

    M64 current_refs() {
      return M64{rbp, Imm32{(uint32_t)(-3 * STACK_VALUE_SIZE)}};
    }

    void reg_move(const R64& dest, const R64& src) {
      if (src != dest)
        assm.mov(dest, src);
    }

    void alive(const R64& reg) {
      debug("alive", reg);
      assert(!live.count(reg));
      live.insert(reg);
    }

    void alive(shared_ptr<Temp> temp) {
      debug("alive", *temp);
      if (temp->reg) {
        if (is_alive(*(temp->reg))) {
          debug("spill", *temp);
          temp->reg = experimental::nullopt;
        } else {
          alive(*(temp->reg));
          reg_temps[*(temp->reg)] = temp;
        }
      }
    }

    bool is_alive(const R64& reg) {
      return live.count(reg);
    }

    void dead(const R64& reg) {
      debug("dead", reg);
      live.erase(reg);
      reg_temps.erase(reg);
    }

    void dead(shared_ptr<Temp> temp) {
      debug("dead", *temp);
      if (temp->reg) {
        dead(*(temp->reg));
      }
    }

    void reserve(const R64& reg) {
      debug("reserve", reg);
      if (reg_temps.count(reg)) {
        auto temp = reg_temps[reg];
        debug("flush", *temp);
        assm.mov(temp_mem(temp->num), reg);
        temp->reg = experimental::nullopt;
        reg_temps.erase(reg);
      } else {
        alive(reg);
      }
    }

    R64 alloc_reg() {
      for (auto reg : r64s) {
        if (!is_alive(reg)) {
          debug("alloc", reg);
          alive(reg);
          return reg;
        }
      }
      for (auto reg : r64s) {
        if (reg_temps.count(reg)) {
          reserve(reg);
          return reg;
        }
      }
      throw RegistersExhausted("alloc_reg");
    }

    M64 temp_mem(size_t i) {
      return M64{rbp, Imm32{(uint32_t)(-(i+RESERVED_STACK_SPACE) * STACK_VALUE_SIZE)}};
    }

    void assign_mem_to_temp(shared_ptr<Temp> dest, M64 base, int num) {
      alive(dest);
      if (dest->reg) {
        auto reg = *(dest->reg);
        assm.mov(reg, base);
        assm.mov(reg, M64{reg, Imm32{STACK_VALUE_SIZE*num}});
      } else {
        auto reg = alloc_reg();
        assm.mov(reg, base);
        assm.mov(reg, M64{reg, Imm32{STACK_VALUE_SIZE*num}});
        assm.mov(temp_mem(dest->num), reg);
        dead(reg);
      }
    }

    void store_temp_to_mem(shared_ptr<Temp> src, M64 base, int num) {
      auto reg = read_temp(src);
      auto r2 = alloc_reg();
      assm.mov(r2, base);
      assm.mov(M64{r2, Imm32{STACK_VALUE_SIZE*num}}, reg);
      dead(reg);
      dead(r2);
    }

    void assign_helper_call_to_temp(shared_ptr<Temp> dest, void* helper, int num) {
      prepare_call_helper(2);
      auto reg = rdi;
      auto r2 = rsi;
      assm.mov(reg, current_closure());
      assm.mov(r2, Imm64{num});
      call_helper(helper, reg, r2);
      write_temp(dest, rax);
      dead(reg);
      dead(r2);
    }

    void assign_local(shared_ptr<Var> src, shared_ptr<Temp> dest) {
      assign_mem_to_temp(dest, current_locals(), src->num);
    }

    void store_local(shared_ptr<Temp> src, shared_ptr<Var> dest) {
      store_temp_to_mem(src, current_locals(), dest->num);
    }

    void assign_ref(shared_ptr<Ref> src, shared_ptr<Temp> dest) {
      assign_mem_to_temp(dest, current_refs(), src->num);
    }

    void assign_deref(shared_ptr<Deref> src, shared_ptr<Temp> dest) {
      prepare_call_helper(1);
      auto reg = rdi;
      assm.mov(reg, current_refs());
      assm.mov(reg, M64{reg, Imm32{STACK_VALUE_SIZE*src->num}});
      call_helper((void*) &helper_read_reference, reg);
      write_temp(dest, rax);
      dead(reg);
    }

    void store_deref(shared_ptr<Temp> src, shared_ptr<Deref> dest) {
      prepare_call_helper(2);
      auto reg = rdi;
      assm.mov(reg, current_refs());
      assm.mov(reg, M64{reg, Imm32{STACK_VALUE_SIZE*dest->num}});
      auto r2 = read_temp(src, rsi);
      call_helper((void*) &helper_write_reference, reg, r2);
      dead(reg);
      dead(r2);
    }

    void assign_glob(shared_ptr<Glob> src, shared_ptr<Temp> dest) {
      assign_helper_call_to_temp(dest, (void *)(&helper_read_global), src->num);
    }

    void store_glob(shared_ptr<Temp> src, shared_ptr<Glob> dest) {
      prepare_call_helper(3);
      auto reg = rdi;
      assm.mov(reg, current_closure());
      auto r2 = rsi;
      assm.mov(r2, Imm64{dest->num});
      auto r3 = read_temp(src, rdx);
      call_helper((void *)(&helper_write_global), reg, r2, r3);
      dead(reg);
      dead(r2);
      dead(r3);
    }

    void assign_function(shared_ptr<IR::Function> src, shared_ptr<Temp> dest) {
      assign_helper_call_to_temp(dest, (void *)(&helper_read_function), src->num);
    }

    R64 read_temp(shared_ptr<Temp> temp, const R64& reg_hint) {
      if (temp->reg) {
        return *(temp->reg);
      } else {
        assm.mov(reg_hint, temp_mem(temp->num));
        return reg_hint;
      }
    }

    R64 read_temp(shared_ptr<Temp> temp) {
      if (temp->reg) {
        return *(temp->reg);
      } else {
        auto reg = alloc_reg();
        assm.mov(reg, temp_mem(temp->num));
        return reg;
      }
    }

    void write_temp(shared_ptr<Temp> temp, const R64& reg) {
      alive(temp);
      if (temp->reg) {
        reg_move(*(temp->reg), reg);
      } else {
        assm.mov(temp_mem(temp->num), reg);
      }
    }

    void write_temp(shared_ptr<Temp> temp, uint64_t cons) {
      alive(temp);
      if (temp->reg) {
         assm.mov(*(temp->reg), Imm64{cons});
      } else {
        auto scratch = alloc_reg();
        assm.mov(scratch, Imm64{cons});
        assm.mov(temp_mem(temp->num), scratch);
        dead(scratch);
      }
    }

    void prepare_call_helper(size_t argc) {
      if (argc > 4) {
        throw InvalidNumArgs(to_string(argc));
      }

      for (size_t i = 0; i < argc; i++) {
        reserve(arg_regs[i]);
      }
    }

    void call_helper(void* fn, const R64 args[], size_t argc) {
      auto scratch = alloc_reg();

      for (size_t i = 0; i < argc; i++) {
        assert(is_alive(arg_regs[i]));
        reg_move(arg_regs[i], args[i]);
        dead(arg_regs[i]);
        dead(args[i]);
      }

      assm.mov(scratch, Imm64{(uint64_t)fn});
      dead(scratch);

      for (auto reg : caller_saved_regs) {
        if (is_alive(reg)) {
          assm.push(reg);
        }
      }

      assm.call(scratch);

      for (auto rit = caller_saved_regs.rbegin(); rit != caller_saved_regs.rend(); ++rit) {
        if (is_alive(*rit)) {
          assm.pop(*rit);
        }
      }
    }

    void call_helper(void* fn) {
      const R64 args[] = {};
      call_helper(fn, args, 0);
    }

    void call_helper(void* fn, const R64 arg1) {
      const R64 args[] = {arg1};
      call_helper(fn, args, 1);
    }

    void call_helper(void* fn, const R64 arg1, const R64 arg2) {
      const R64 args[] = {arg1, arg2};
      call_helper(fn, args, 2);
    }

    void call_helper(void* fn, const R64 arg1, const R64 arg2, const R64 arg3) {
      const R64 args[] = {arg1, arg2, arg3};
      call_helper(fn, args, 3);
    }

    void call_helper(void* fn, const R64 arg1, const R64 arg2, const R64 arg3, const R64 arg4) {
      const R64 args[] = {arg1, arg2, arg3, arg4};
      call_helper(fn, args, 4);
    }

    void preamble() {
      // preconditions:
      // rdi contains a pointer to the closure (not a tagged pointer)
      // rsi contains a pointer to the list of local variables
      // rdx contains a pointer to the list of references (local references and free vars)
      // Stuff that needs to happen:
      // Extend the stack RESERVED_STACK_SPACE + num_temps*8 downward
      // Store closure pointer into the special place
      // Store list of locals into the special place
      // Store references list into the special place

      // Push all the registers we're gonna use

      assm.push(rbp);
      assm.mov(rbp, rsp);

      assm.sub(rsp, Imm32{((uint32_t)num_temps + RESERVED_STACK_SPACE)*STACK_VALUE_SIZE});
      assm.mov(current_closure(), rdi);
      assm.mov(current_locals(), rsi);
      assm.mov(current_refs(), rdx);

      assm.push(rbx);
      assm.push(r12);
      assm.push(r13);
      assm.push(r14);
      assm.push(r15);
    }

    void postamble(const R64& retval) {
      assm.mov(rax, retval);
      assm.pop(r15);
      assm.pop(r14);
      assm.pop(r13);
      assm.pop(r12);
      assm.pop(rbx);
      assm.leave();
    }

    void compile(IR::InstructionList& ir, x64asm::Function& function) {
      assm.start(function);

      alive(rax);
      alive(rbp);
      alive(rsp);

      preamble();

      for (auto instruction : ir) {
        #ifdef DEBUG
          assm.nop();
        #endif
        debug("--");
        function.reserve(function.size() + IR_INSTRUCTION_BYTE_UPPER_BOUND);
        switch (instruction->op()) {
          case IR::Operation::Noop: {
            break;
          }
          case IR::Operation::OutputLabel: {
            auto ol = dynamic_cast<OutputLabel*>(instruction);
            assm.bind(x64asm::Label{ol->label->toString()});
            break;
          }
          case IR::Operation::Assign: {
            if (auto assign = dynamic_cast<Assign<Var>*>(instruction)) {
              assign_local(assign->src, assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Const>*>(instruction)) {
              write_temp(assign->dest, assign->src->val);
            } else if (auto assign = dynamic_cast<Assign<RetVal>*>(instruction)) {
              write_temp(assign->dest, rax);
            } else if (auto assign = dynamic_cast<Assign<Temp>*>(instruction)) {
              auto s1 = read_temp(assign->src);
              write_temp(assign->dest, s1);
              dead(s1);
            } else if (auto assign = dynamic_cast<Assign<Ref>*>(instruction)) {
              assign_ref(assign->src, assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Deref>*>(instruction)) {
              assign_deref(assign->src, assign->dest);
            } else if (auto assign = dynamic_cast<Assign<Glob>*>(instruction)) {
              assign_glob(assign->src, assign->dest);
            } else if (auto assign = dynamic_cast<Assign<IR::Function>*>(instruction)) {
              assign_function(assign->src, assign->dest);
            }
            break;
          }
          case IR::Operation::Store: {
            if (auto store = dynamic_cast<Store<Var>*>(instruction)) {
              store_local(store->src, store->dest);
            } else if (auto store = dynamic_cast<Store<Deref>*>(instruction)) {
              store_deref(store->src, store->dest);
            } else if (auto store = dynamic_cast<Store<Glob>*>(instruction)) {
              store_glob(store->src, store->dest);
            }
            break;
          }
          case IR::Operation::Add: {
            auto add = dynamic_cast<Add*>(instruction);
            prepare_call_helper(2);
            auto s1 = read_temp(add->src1, rdi);
            auto s2 = read_temp(add->src2, rsi);
            call_helper((void *)(&helper_add), s1, s2);
            dead(s1);
            dead(s2);
            write_temp(add->dest, rax);
            break;
          }
          case IR::Operation::IntAdd: {
            auto intadd = dynamic_cast<IntAdd*>(instruction);
            auto s1 = read_temp(intadd->src1);
            auto s2 = read_temp(intadd->src2);
            assm.add(s2, s1);
            dead(s1);
            write_temp(intadd->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Sub: {
            auto sub = dynamic_cast<Sub*>(instruction);
            auto s1 = read_temp(sub->src1);
            auto s2 = read_temp(sub->src2);
            assm.sub(s2, s1);
            dead(s1);
            write_temp(sub->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Mul: {
            auto mul = dynamic_cast<Mul*>(instruction);
            auto s1 = read_temp(mul->src1);
            auto s2 = read_temp(mul->src2);
            assm.imul(s2, s1);
            dead(s1);
            assm.sar(s2, Imm8{3});
            write_temp(mul->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Div: {
            auto div = dynamic_cast<Div*>(instruction);
            reserve(rdx);
            auto s1 = read_temp(div->src1);
            auto s2 = read_temp(div->src2);
            reg_move(rax, s1);
            dead(s1);
            assm.cqo();
            assm.idiv(s2);
            dead(s2);
            dead(rdx);
            assm.sal(rax, Imm8{3});
            write_temp(div->dest, rax);
            break;
          }
          case IR::Operation::Gt: {
            auto gt = dynamic_cast<Gt*>(instruction);
            auto s1 = read_temp(gt->src1);
            auto s2 = read_temp(gt->src2);
            assm.cmp(s2, s1);
            assm.mov(s2, Imm64{_BOOLEAN_TAG});
            assm.mov(s1, Imm64{0b1000 | _BOOLEAN_TAG});
            assm.cmovg(s2, s1);
            dead(s1);
            write_temp(gt->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Geq: {
            auto gte = dynamic_cast<Geq*>(instruction);
            auto s1 = read_temp(gte->src1);
            auto s2 = read_temp(gte->src2);
            assm.cmp(s2, s1);
            assm.mov(s2, Imm64{_BOOLEAN_TAG});
            assm.mov(s1, Imm64{0b1000 | _BOOLEAN_TAG});
            assm.cmovge(s2, s1);
            dead(s1);
            write_temp(gte->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Eq: {
            auto eq = dynamic_cast<Eq*>(instruction);
            prepare_call_helper(2);
            auto s1 = read_temp(eq->src1, rdi);
            auto s2 = read_temp(eq->src2, rsi);
            call_helper((void *)(&helper_equals), s1, s2);
            dead(s1);
            dead(s2);
            write_temp(eq->dest, rax);
            break;
          }
          case IR::Operation::FastEq: {
            auto eq = dynamic_cast<FastEq*>(instruction);
            auto s1 = read_temp(eq->src1);
            auto s2 = read_temp(eq->src2);
            assm.cmp(s2, s1);
            assm.mov(s2, Imm64{_BOOLEAN_TAG});
            assm.mov(s1, Imm64{0b1000 | _BOOLEAN_TAG});
            assm.cmove(s2, s1);
            dead(s1);
            write_temp(eq->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Neg: {
            auto neg = dynamic_cast<Neg*>(instruction);
            auto s1 = read_temp(neg->src);
            assm.neg(s1);
            write_temp(neg->dest, s1);
            dead(s1);
            break;
          }
          case IR::Operation::Not: {
            auto nott = dynamic_cast<Not*>(instruction);
            auto s1 = read_temp(nott->src);
            assm.xor_(s1, Imm32{0b1000});
            write_temp(nott->dest, s1);
            dead(s1);
            break;
          }
          case IR::Operation::ShortJump: {
            auto sj = dynamic_cast<ShortJump*>(instruction);
            assm.jmp(x64asm::Label{sj->label->toString()});
            break;
          }
          case IR::Operation::Jump: {
            auto jump = dynamic_cast<Jump*>(instruction);
            assm.jmp_1(x64asm::Label{jump->label->toString()});
            break;
          }
          case IR::Operation::CondJump: {
            auto cjump = dynamic_cast<CondJump*>(instruction);
            auto s1 = read_temp(cjump->cond);
            assm.cmp(s1, Imm32{0b1000 | _BOOLEAN_TAG});
            dead(s1);
            assm.je_1(x64asm::Label{cjump->label->toString()});
            break;
          }
          case IR::Operation::Call: {
            auto call = dynamic_cast<IR::Call*>(instruction);
            prepare_call_helper(3);
            for (int i = call->args.size() - 1; i >= 0; --i) {
              auto s = read_temp(call->args[i]);
              assm.push(s);
              dead(s);
            }
            auto s1 = read_temp(call->closure, rdi);
            auto s2 = alloc_reg();
            auto s3 = rdx;
            assm.mov(s2, rsp);
            assm.mov(s3, Imm32{(uint32_t)call->args.size()});
            call_helper((void *)(&helper_call_function), s1, s2, s3);
            dead(s1);
            dead(s3);
            assm.add(rsp, Imm32{(uint32_t)call->args.size()*STACK_VALUE_SIZE});
            break;
          }
          case IR::Operation::Return: {
            auto ret = dynamic_cast<IR::Return*>(instruction);
            auto s1 = read_temp(ret->val);
            postamble(s1);
            dead(s1);
            assm.ret();
            break;
          }
          case IR::Operation::CallHelper: {
            if (auto op = dynamic_cast<CallHelper<Helper::GarbageCollect>*>(instruction)) {
              prepare_call_helper(0);
              call_helper((void *)(&helper_garbage_collect));
            } else if (auto op = dynamic_cast<CallHelper<Helper::AllocRecord>*>(instruction)) {
              prepare_call_helper(0);
              call_helper((void *)(&helper_alloc_record));
            } else if (auto op = dynamic_cast<CallHelper<Helper::FieldLoad>*>(instruction)) {
              prepare_call_helper(3);
              auto s1 = rdi;
              assm.mov(s1, current_closure());
              auto s2 = read_temp(op->args[0], rsi);
              auto s3 = rdx;
              assm.mov(s3, Imm64{op->arg0});
              call_helper((void *)(&helper_field_load), s1, s2, s3);
              dead(s1);
              dead(s2);
              dead(s2);
            } else if (auto op = dynamic_cast<CallHelper<Helper::FieldStore>*>(instruction)) {
              prepare_call_helper(4);
              auto s1 = rdi;
              assm.mov(s1, current_closure());
              auto s2 = read_temp(op->args[0], rsi);
              auto s3 = rdx;
              assm.mov(s3, Imm64{op->arg0});
              auto s4 = read_temp(op->args[1], rcx);
              call_helper((void *)(&helper_field_store), s1, s2, s3, s4);
              dead(s1);
              dead(s2);
              dead(s3);
              dead(s4);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexLoad>*>(instruction)) {
              prepare_call_helper(2);
              auto s1 = read_temp(op->args[0], rdi);
              auto s2 = read_temp(op->args[1], rsi);
              call_helper((void *)(&helper_index_load), s1, s2);
              dead(s1);
              dead(s2);
            } else if (auto op = dynamic_cast<CallHelper<Helper::IndexStore>*>(instruction)) {
              prepare_call_helper(3);
              auto s1 = read_temp(op->args[0], rdi);
              auto s2 = read_temp(op->args[1], rsi);
              auto s3 = read_temp(op->args[2], rdx);
              call_helper((void *)(&helper_index_store), s1, s2, s3);
              dead(s1);
              dead(s2);
              dead(s3);
            }
            break;
          }
          case IR::Operation::CallAssert: {
            if (auto op = dynamic_cast<CallAssert<Assert::AssertInt>*>(instruction)) {
              prepare_call_helper(1);
              auto s1 = read_temp(op->arg, rdi);
              call_helper((void *)(&helper_assert_int), s1);
              dead(s1);
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertNotZero>*>(instruction)) {
              prepare_call_helper(1);
              auto s1 = read_temp(op->arg, rdi);
              call_helper((void *)(&helper_assert_not_zero), s1);
              dead(s1);
            } else if (auto op = dynamic_cast<CallAssert<Assert::AssertBool>*>(instruction)) {
              prepare_call_helper(1);
              auto s1 = read_temp(op->arg, rdi);
              call_helper((void *)(&helper_assert_bool), s1);
              dead(s1);
            }
            break;
          }
          case IR::Operation::AllocClosure: {
            auto op = dynamic_cast<AllocClosure*>(instruction);

            prepare_call_helper(1);
            auto s1 = read_temp(op->function, rdi);
            call_helper((void*) &helper_convert_to_closure, s1);
            dead(s1);

            for (shared_ptr<Temp> t : op->refs) {
              prepare_call_helper(2);
              assm.mov(rdi, rax);
              auto s2 = read_temp(t, rsi);
              call_helper((void*) &helper_add_reference_to_closure, rdi, s2);
              dead(s2);
            }
            break;
          }
          case IR::Operation::And: {
            auto andd = dynamic_cast<And*>(instruction);
            auto s1 = read_temp(andd->src1);
            auto s2 = read_temp(andd->src2);
            assm.and_(s2, s1);
            dead(s1);
            write_temp(andd->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Or: {
            auto orr = dynamic_cast<Or*>(instruction);
            auto s1 = read_temp(orr->src1);
            auto s2 = read_temp(orr->src2);
            assm.or_(s2, s1);
            dead(s1);
            write_temp(orr->dest, s2);
            dead(s2);
            break;
          }
          case IR::Operation::Fork: {
            auto fork = dynamic_cast<Fork*>(instruction);
            auto s1 = read_temp(fork->src);
            write_temp(fork->dest1, s1);
            write_temp(fork->dest2, s1);
            dead(s1);
            break;
          }
          default: {
            throw UnexpectedOperation(to_string(static_cast<int>(instruction->op())));
          }
        }
      }

      dead(rax);
      dead(rbp);
      dead(rsp);

      assm.finish();
    }

  public:
    Compiler(IR::InstructionList& ir, size_t num_temps) : ir(ir), num_temps(num_temps) {}

    void compileInto(x64asm::Function& func) {
      compile(ir, func);
    }
  };
}
