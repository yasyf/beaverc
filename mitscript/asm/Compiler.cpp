#include "Compiler.h"

namespace ASM {
  M64 Compiler::current_closure() {
    return M64{rbp, Imm32{(uint32_t)(-1 * STACK_VALUE_SIZE)}};
  }

  M64 Compiler::current_locals() {
    return M64{rbp, Imm32{(uint32_t)(-2 * STACK_VALUE_SIZE)}};
  }

  M64 Compiler::current_refs() {
    return M64{rbp, Imm32{(uint32_t)(-3 * STACK_VALUE_SIZE)}};
  }

  void Compiler::reg_move(const R64& dest, const R64& src) {
    if (src != dest)
      assm.mov(dest, src);
  }

  void Compiler::alive(const R64& reg, bool is_shared) {
    debug("alive", reg);
    assert(!live.count(reg.hash()));
    live.insert(reg.hash());
    if (is_shared)
      shared.insert(reg.hash());
  }

  void Compiler::alive(shared_ptr<Temp> temp) {
    if (temp->reg) {
      debug("alive", *temp);
      if (is_allocated(*(temp->reg))) {
        debug("spill", *temp);
        temp->reg = nullopt;
      } else if (!is_alive(*(temp->reg))) {
        alive(*(temp->reg), temp->shared_reg);
      }
    }
  }

  void Compiler::alive(shared_ptr<Var> var, bool load) {
    if (var->reg && !reg_vars.count(var->num)) {
      debug("alive", *var);
      if (is_allocated(*(var->reg))) {
        debug("spill", *var);
        var->reg = nullopt;
      } else {
        reg_vars[var->num] = var;
        if (load)
          assign_mem_to_reg(*(var->reg), current_locals(), var->num);
        if (!is_alive(*(var->reg)))
          alive(*(var->reg), true);
      }
    }
  }

  bool Compiler::is_alive(const R64& reg) {
    return live.count(reg.hash());
  }

  bool Compiler::is_allocated(const R64& reg) {
    return live.count(reg.hash()) && !shared.count(reg.hash());
  }

  bool Compiler::is_alive(shared_ptr<Var> var) {
    return reg_vars.count(var->num);
  }

  void Compiler::dead(const R64& reg, bool is_shared = false) {
    if (shared.count(reg.hash()) && !is_shared)
      return;
    debug("dead", reg);
    live.erase(reg.hash());
    shared.erase(reg.hash());
  }

  void Compiler::dead(shared_ptr<Temp> temp) {
    if (temp->reg) {
      debug("dead", *temp);
      if (temp->isVar()) {
        if (dead(reg_vars[temp->getVar()])) {
          reg_vars.erase(temp->getVar());
        }
      } else {
        dead(*(temp->reg));
      }
    }
  }

  bool Compiler::dead(shared_ptr<Var> var) {
    if (var->reg && ir_count >= var->live_end) {
      debug("dead", *var);
      dead(*(var->reg), true);
      return true;
    }
    return false;
  }

  void Compiler::reserve(const R64& reg) {
    alive(reg, false);
  }

  R64 Compiler::alloc_reg() {
    for (auto reg : reg_pool) {
      if (!is_alive(reg)) {
        debug("alloc", reg);
        reserve(reg);
        return reg;
      }
    }
    throw RegistersExhausted("alloc_reg");
  }

  M64 Compiler::temp_mem(size_t i) {
    return M64{rbp, Imm32{(uint32_t)(-(i+RESERVED_STACK_SPACE) * STACK_VALUE_SIZE)}};
  }

  void Compiler::assign_mem_to_reg(const R64& dest, const M64& base, int num) {
    assm.mov(dest, base);
    assm.mov(dest, M64{dest, Imm32{(uint32_t)(STACK_VALUE_SIZE*num)}});
  }

  void Compiler::assign_reg_to_mem(const R64& src, const M64& base, int num) {
    auto r2 = alloc_reg();
    assm.mov(r2, base);
    assm.mov(M64{r2, Imm32{(uint32_t)(STACK_VALUE_SIZE*num)}}, src);
    dead(r2);
  }

  void Compiler::assign_mem_to_temp(shared_ptr<Temp> dest, const M64& base, int num) {
    alive(dest);
    if (dest->reg) {
      assign_mem_to_reg(*(dest->reg), base, num);
    } else {
      auto reg = alloc_reg();
      assign_mem_to_reg(reg, base, num);
      assm.mov(temp_mem(dest->num), reg);
      dead(reg);
    }
  }

  void Compiler::store_temp_to_mem(shared_ptr<Temp> src, const M64& base, int num) {
    auto reg = read_temp(src);
    auto r2 = alloc_reg();
    assm.mov(r2, base);
    assm.mov(M64{r2, Imm32{(uint32_t)(STACK_VALUE_SIZE*num)}}, reg);
    dead(reg);
    dead(r2);
  }

  void Compiler::assign_helper_call_to_temp(shared_ptr<Temp> dest, void* helper, int num) {
    prepare_call_helper(2);
    auto reg = rdi;
    auto r2 = rsi;
    assm.mov(reg, current_closure());
    assm.mov(r2, Imm64{(uint64_t) num});
    call_helper(helper, reg, r2);
    write_temp(dest, rax);
    dead(reg);
    dead(r2);
  }

  void Compiler::assign_local(shared_ptr<Var> src, shared_ptr<Temp> dest) {
    alive(src, true);
    if (src->reg) {
      write_temp(dest, *(src->reg));
    } else {
      assign_mem_to_temp(dest, current_locals(), src->num);
    }
  }

  void Compiler::store_local(shared_ptr<Temp> src, shared_ptr<Var> dest) {
    if (is_alive(dest)) {
      dead(dest);
    } else {
      alive(dest, false);
    }
    if (dest->reg) {
      auto reg = read_temp(src);
      reg_move(*(dest->reg), reg);
      dead(reg);
      dest->dirty = true;
    } else {
      store_temp_to_mem(src, current_locals(), dest->num);
    }
  }

  void Compiler::assign_ref(shared_ptr<Ref> src, shared_ptr<Temp> dest) {
    assign_mem_to_temp(dest, current_refs(), src->num);
  }

  void Compiler::assign_deref(shared_ptr<Deref> src, shared_ptr<Temp> dest) {
    prepare_call_helper(1);
    auto reg = rdi;
    assm.mov(reg, current_refs());
    assm.mov(reg, M64{reg, Imm32{(uint32_t)(STACK_VALUE_SIZE*src->num)}});
    call_helper((void*) &helper_read_reference, reg);
    write_temp(dest, rax);
    dead(reg);
  }

  void Compiler::store_deref(shared_ptr<Temp> src, shared_ptr<Deref> dest) {
    prepare_call_helper(2);
    auto reg = rdi;
    assm.mov(reg, current_refs());
    assm.mov(reg, M64{reg, Imm32{(uint32_t)(STACK_VALUE_SIZE*dest->num)}});
    auto r2 = read_temp(src, rsi);
    call_helper((void*) &helper_write_reference, reg, r2);
    dead(reg);
    dead(r2);
  }

  void Compiler::assign_glob(shared_ptr<Glob> src, shared_ptr<Temp> dest) {
    assign_helper_call_to_temp(dest, (void *)(&helper_read_global), src->num);
  }

  void Compiler::store_glob(shared_ptr<Temp> src, shared_ptr<Glob> dest) {
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

  void Compiler::assign_function(shared_ptr<IR::Function> src, shared_ptr<Temp> dest) {
    assign_helper_call_to_temp(dest, (void *)(&helper_read_function), src->num);
  }

  void Compiler::extract_bits(shared_ptr<Temp> temp, const R64& dest, size_t start, size_t length) {
    assm.mov(dest, Imm64{start + (length << 8)});
    if (temp->reg) {
      assm.bextr(dest, *(temp->reg), dest);
    } else {
      assm.bextr(dest, temp_mem(temp->num), dest);
    }
  }

  R64 Compiler::read_temp(shared_ptr<Temp> temp, optional<R64> reg_hint, bool scratch) {
    if (temp->reg) {
      if (scratch && temp->shared_reg) {
        auto reg = (reg_hint && !is_alive(*reg_hint)) ? *reg_hint : alloc_reg();
        assm.mov(reg, *(temp->reg));
        return reg;
      } else {
        return *(temp->reg);
      }
    } else {
      auto reg = reg_hint ? *reg_hint : alloc_reg();
      assm.mov(reg, temp_mem(temp->num));
      return reg;
    }
  }

  // I have no idea why this is needed, but -O2 segfaults without it
  R64 Compiler::read_temp(shared_ptr<Temp> temp, const R64& reg_hint, bool scratch) {
    return read_temp(temp, optional<R64>(reg_hint), scratch);
  }

  void Compiler::write_temp(shared_ptr<Temp> temp, const R64& reg) {
    alive(temp);
    if (temp->reg) {
      reg_move(*(temp->reg), reg);
    } else {
      assm.mov(temp_mem(temp->num), reg);
    }
  }

  void Compiler::write_temp(shared_ptr<Temp> temp, uint64_t cons) {
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

  void Compiler::flush_vars() {
    for (auto const& p : reg_vars) {
      if (p.second->dirty) {
        p.second->dirty = false;
        assign_reg_to_mem(*(p.second->reg), current_locals(), p.first);
      }
    }
  }

  void Compiler::check_vars() {
    for (auto it = reg_vars.begin(); it != reg_vars.end(); ) {
      if (dead(it->second)) {
        it = reg_vars.erase(it);
      } else {
        ++it;
      }
    }
  }

  void Compiler::prepare_call_helper(size_t argc) {
    if (argc > 4) {
      throw InvalidNumArgs(to_string(argc));
    }

    for (size_t i = 0; i < argc; i++) {
      reserve(arg_regs[i]);
    }
  }

  void Compiler::call_helper(void* fn, const R64 args[], size_t argc) {
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

  void Compiler::call_helper(void* fn) {
    const R64 args[] = {};
    call_helper(fn, args, 0);
  }

  void Compiler::call_helper(void* fn, const R64 arg1) {
    const R64 args[] = {arg1};
    call_helper(fn, args, 1);
  }

  void Compiler::call_helper(void* fn, const R64 arg1, const R64 arg2) {
    const R64 args[] = {arg1, arg2};
    call_helper(fn, args, 2);
  }

  void Compiler::call_helper(void* fn, const R64 arg1, const R64 arg2, const R64 arg3) {
    const R64 args[] = {arg1, arg2, arg3};
    call_helper(fn, args, 3);
  }

  void Compiler::call_helper(void* fn, const R64 arg1, const R64 arg2, const R64 arg3, const R64 arg4) {
    const R64 args[] = {arg1, arg2, arg3, arg4};
    call_helper(fn, args, 4);
  }

  void Compiler::preamble() {
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

  void Compiler::postamble(const R64& retval) {
    assm.mov(rax, retval);
    assm.pop(r15);
    assm.pop(r14);
    assm.pop(r13);
    assm.pop(r12);
    assm.pop(rbx);
    assm.leave();
  }

  void Compiler::compile(IR::InstructionList& ir, x64asm::Function& function) {
    assm.start(function);

    preamble();

    function.reserve(function.size() + IR_INSTRUCTION_BYTE_UPPER_BOUND * ir.size());

    for (auto instruction : ir) {
      #ifdef DEBUG
        assm.nop();
      #endif
      debug("--");
      switch (instruction->op()) {
        case IR::Operation::ForceLoad: {
          if (auto force = dynamic_cast<ForceLoad<Var>*>(instruction)) {
            alive(force->src, true);
          }
          break;
        }
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
          auto s2 = read_temp(intadd->src2, intadd->dest->reg, true);
          assm.add(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(intadd->dest, s2);
          break;
        }
        case IR::Operation::Sub: {
          auto sub = dynamic_cast<Sub*>(instruction);
          auto s1 = read_temp(sub->src1);
          auto s2 = read_temp(sub->src2, sub->dest->reg, true);
          assm.sub(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(sub->dest, s2);
          break;
        }
        case IR::Operation::Mul: {
          auto mul = dynamic_cast<Mul*>(instruction);
          auto s1 = read_temp(mul->src1);
          auto s2 = read_temp(mul->src2, mul->dest->reg, true);
          assm.imul(s2, s1);
          dead(s1);
          assm.sar(s2, Imm8{3});
          dead(s2);
          write_temp(mul->dest, s2);
          break;
        }
        case IR::Operation::Div: {
          auto div = dynamic_cast<Div*>(instruction);
          reserve(rdx);
          reserve(rax);
          auto s1 = read_temp(div->src1);
          auto s2 = read_temp(div->src2);
          reg_move(rax, s1);
          dead(s1);
          assm.cqo();
          assm.idiv(s2);
          dead(s2);
          dead(rdx);
          assm.sal(rax, Imm8{3});
          dead(rax);
          write_temp(div->dest, rax);
          break;
        }
        case IR::Operation::Gt: {
          auto gt = dynamic_cast<Gt*>(instruction);
          auto s1 = read_temp(gt->src1, nullopt, true);
          auto s2 = read_temp(gt->src2, gt->dest->reg, true);
          assm.cmp(s2, s1);
          assm.mov(s2, Imm64{_BOOLEAN_TAG});
          assm.mov(s1, Imm64{0b1000 | _BOOLEAN_TAG});
          assm.cmovg(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(gt->dest, s2);
          break;
        }
        case IR::Operation::Geq: {
          auto gte = dynamic_cast<Geq*>(instruction);
          auto s1 = read_temp(gte->src1, nullopt, true);
          auto s2 = read_temp(gte->src2, gte->dest->reg, true);
          assm.cmp(s2, s1);
          assm.mov(s2, Imm64{_BOOLEAN_TAG});
          assm.mov(s1, Imm64{0b1000 | _BOOLEAN_TAG});
          assm.cmovge(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(gte->dest, s2);
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
          auto s1 = read_temp(eq->src1, nullopt, true);
          auto s2 = read_temp(eq->src2, eq->dest->reg, true);
          assm.cmp(s2, s1);
          assm.mov(s2, Imm64{_BOOLEAN_TAG});
          assm.mov(s1, Imm64{0b1000 | _BOOLEAN_TAG});
          assm.cmove(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(eq->dest, s2);
          break;
        }
        case IR::Operation::Neg: {
          auto neg = dynamic_cast<Neg*>(instruction);
          auto s1 = read_temp(neg->src, neg->dest->reg, true);
          assm.neg(s1);
          dead(s1);
          write_temp(neg->dest, s1);
          break;
        }
        case IR::Operation::Not: {
          auto nott = dynamic_cast<Not*>(instruction);
          auto s1 = read_temp(nott->src, nott->dest->reg, true);
          assm.xor_(s1, Imm32{0b1000});
          dead(s1);
          write_temp(nott->dest, s1);
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
          flush_vars();
          call_helper((void *)(&helper_call_function), s1, s2, s3);
          dead(s1);
          dead(s2);
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
            flush_vars();
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
          x64asm::Label skip;
          auto reg = alloc_reg();
          if (auto op = dynamic_cast<CallAssert<Assert::AssertInt>*>(instruction)) {
            extract_bits(op->arg, reg, 0, 3);
            assm.cmp(reg, Imm32{_INTEGER_TAG});
            dead(reg);
            assm.je_1(skip);
            call_helper((void *)(&helper_throw_not_int));
          } else if (auto op = dynamic_cast<CallAssert<Assert::AssertNotZero>*>(instruction)) {
            auto s1 = read_temp(op->arg, reg);
            assm.cmp(s1, Imm32{_INTEGER_TAG});
            dead(s1);
            assm.jne_1(skip);
            call_helper((void *)(&helper_throw_zero));
          } else if (auto op = dynamic_cast<CallAssert<Assert::AssertBool>*>(instruction)) {
            extract_bits(op->arg, reg, 0, 3);
            assm.cmp(reg, Imm32{_BOOLEAN_TAG});
            dead(reg);
            assm.je_1(skip);
            call_helper((void *)(&helper_throw_not_bool));
          }
          assm.bind(skip);
          break;
        }
        case IR::Operation::AllocClosure: {
          auto op = dynamic_cast<AllocClosure*>(instruction);
          reserve(rax);

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

          dead(rax);
          break;
        }
        case IR::Operation::And: {
          auto andd = dynamic_cast<And*>(instruction);
          auto s1 = read_temp(andd->src1);
          auto s2 = read_temp(andd->src2, andd->dest->reg, true);
          assm.and_(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(andd->dest, s2);
          break;
        }
        case IR::Operation::Or: {
          auto orr = dynamic_cast<Or*>(instruction);
          auto s1 = read_temp(orr->src1);
          auto s2 = read_temp(orr->src2, orr->dest->reg, true);
          assm.or_(s2, s1);
          dead(s1);
          dead(s2);
          write_temp(orr->dest, s2);
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

      check_vars();
      ir_count++;
    }

    assm.finish();
  }

  Compiler::Compiler(IR::InstructionList& ir, size_t num_temps) : ir(ir), num_temps(num_temps) {}

  void Compiler::compileInto(x64asm::Function& func) {
    compile(ir, func);
  }
}
