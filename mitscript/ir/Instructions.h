#pragma once

#include "../bccompiler/Types.h"
#include "../vm/Value.h"
#include <vector>

using namespace std;

namespace IR {
  struct Operand {
    virtual string toString() = 0;
  };

  struct Label {
    size_t num;

    Label(size_t num) : num(num) {}
    virtual string toString() { return "l" + to_string(num); }
  };

  struct Temp : Operand {
    size_t num;

    Temp(size_t num) : num(num) {}
    virtual string toString() { return "t" + to_string(num); }
  };

  struct RetVal : Operand {
    virtual string toString() { return "retval"; }
  };

  struct Var : Operand {
    size_t num;

    Var(size_t num) : num(num) {}
    virtual string toString() { return "%" + to_string(num); }
  };

  struct Glob : Operand {
    size_t num;

    Glob(size_t num) : num(num) {}
    virtual string toString() { return "%%" + to_string(num); }
  };

  struct Function : Operand {
    size_t num;

    Function(size_t num) : num(num) {}
    virtual string toString() { return "f" + to_string(num); }
  };

  struct Ref : Operand {
    size_t num;

    Ref(size_t num) : num(num) {}
    virtual string toString() { return "r" + to_string(num); }
  };

  struct Deref : Operand {
    size_t num;

    Deref(size_t num) : num(num) {}
    virtual string toString() { return "*r" + to_string(num); }
  };

  struct Const : Operand {
    uint64_t val;

    Const(uint64_t val) : val(val) {}

    Const(std::shared_ptr<BC::Constant> constant) {
      if (auto val = dynamic_pointer_cast<BC::Integer>(constant))
        this->val = VM::Value::makeInteger(val->value).value;
      else if (auto val = dynamic_pointer_cast<BC::None>(constant))
        this->val = VM::Value::makeNone().value;
      else if (auto val = dynamic_pointer_cast<BC::Boolean>(constant))
        this->val = VM::Value::makeBoolean(val->value).value;
      else if (auto val = dynamic_pointer_cast<BC::String>(constant))
        this->val = VM::Value::makeStringConstant(val->value.c_str()).value;
      else
        throw "invalid Constant!";
    }

    virtual string toString() { return "$" + to_string(val); }
  };

  enum class Operation {
    Assign,
    Store,
    Add,
    Sub,
    Mul,
    Div,
    Gt,
    Geq,
    Eq,
    And,
    Or,
    Not,
    Neg,
    Call,
    AllocClosure,
    Return,
    OutputLabel,
    Jump,
    CondJump,
    CallHelper,
  };

  enum class Helper {
    GarbageCollect,
    AllocRecord,
    FieldLoad,
    FieldStore,
    IndexLoad,
    IndexStore,
    AssertInt,
    AssertNotZero,
    AssertBool,
  };

  struct Instruction {
    virtual string toString() = 0;
    virtual Operation op() = 0;
  };

  template<typename S>
  struct Assign : Instruction {
    Temp dest;
    S src;

    Assign(Temp dest, S src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::Assign; }
    virtual string toString() { return dest.toString() + " = " + src.toString(); }
  };

  template<typename D>
  struct Store : Instruction {
    D dest;
    Temp src;

    Store(D dest, Temp src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::Store; }
    virtual string toString() { return dest.toString() + " = " + src.toString(); }
  };

  template<Operation Op>
  struct BinOp : Instruction {
    Temp dest;
    Temp src1;
    Temp src2;

    BinOp(Temp dest, Temp src1, Temp src2) : dest(dest), src1(src1), src2(src2) {}
    virtual Operation op() { return Op; }
    virtual string opString() = 0;
    virtual string toString() { return dest.toString() + " = " + src1.toString() + " " + opString() + " " + src2.toString(); }
  };

  struct Add : BinOp<Operation::Add> {
    using BinOp::BinOp;
    virtual string opString() { return "+"; }
  };

  struct Sub : BinOp<Operation::Sub> {
    using BinOp::BinOp;
    virtual string opString() { return "-"; }
  };

  struct Mul : BinOp<Operation::Mul> {
    using BinOp::BinOp;
    virtual string opString() { return "*"; }
  };

  struct Div : BinOp<Operation::Div> {
    using BinOp::BinOp;
    virtual string opString() { return "/"; }
  };

  struct Gt : BinOp<Operation::Gt> {
    using BinOp::BinOp;
    virtual string opString() { return "<"; }
  };

  struct Geq : BinOp<Operation::Geq> {
    using BinOp::BinOp;
    virtual string opString() { return "<="; }
  };

  struct Eq : BinOp<Operation::Eq> {
    using BinOp::BinOp;
    virtual string opString() { return "=="; }
  };

  struct And : BinOp<Operation::And> {
    using BinOp::BinOp;
    virtual string opString() { return "&"; }
  };

  struct Or : BinOp<Operation::Or> {
    using BinOp::BinOp;
    virtual string opString() { return "|"; }
  };

  template<Operation Op>
  struct UnOp : Instruction {
    Temp dest;
    Temp src;

    UnOp(Temp dest, Temp src) : dest(dest), src(src) {}
    virtual Operation op() { return Op; }
    virtual string opString() = 0;
    virtual string toString() { return dest.toString() + " = " + opString() + src.toString(); }
  };

  struct Neg : UnOp<Operation::Neg> {
    using UnOp::UnOp;
    virtual string opString() { return "-"; }
  };

  struct Not : UnOp<Operation::Not> {
    using UnOp::UnOp;
    virtual string opString() { return "!"; }
  };

  struct Call : Instruction {
    Temp closure;
    vector<Temp> args;

    Call(Temp closure, vector<Temp> args) : closure(closure), args(args) {}
    virtual Operation op() { return Operation::Call; }
    virtual string toString() { return "call " + closure.toString(); }
  };

  struct AllocClosure : Instruction {
    Temp function;
    vector<Temp> refs;

    AllocClosure(Temp function, vector<Temp> refs) : function(function), refs(refs) {}
    virtual Operation op() { return Operation::AllocClosure; }
    virtual string toString() { return "alloc_closure " + function.toString(); }
  };

  template<Helper H>
  struct CallHelper : Instruction {
    size_t arg0;
    vector<Temp> args;

    CallHelper() {}

    CallHelper(Temp arg) {
      this->args = {arg};
    }

    CallHelper(size_t arg0) {
      this->arg0 = arg0;
    }

    CallHelper(size_t arg0, Temp arg) {
      this->arg0 = arg0;
      this->args = {arg};
    }

    CallHelper(Temp arg1, Temp arg2) {
      this->args = {arg1, arg2};
    }

    CallHelper(size_t arg0, Temp arg1, Temp arg2) {
      this->arg0 = arg0;
      this->args = {arg1, arg2};
    }

    CallHelper(Temp arg1, Temp arg2, Temp arg3) {
      this->args = {arg1, arg2, arg3};
    }

    static Helper helper() { return H; }
    virtual Operation op() { return Operation::CallHelper; }
    virtual string toString() { return "call_helper " + to_string(static_cast<int>(H)); }
  };


  struct Return : Instruction {
    Temp val;

    Return(Temp val) : val(val) {}
    virtual Operation op() { return Operation::Return; }
    virtual string toString() { return "return " + val.toString(); }
  };

  struct OutputLabel : Instruction {
    Label label;

    OutputLabel(Label label) : label(label) {}
    virtual Operation op() { return Operation::OutputLabel; }
    virtual string toString() { return label.toString() + ":"; }
  };

  struct Jump : Instruction {
    Label label;

    Jump(Label label) : label(label) {}
    virtual Operation op() { return Operation::Jump; }
    virtual string toString() { return "jmp " + label.toString(); }
  };

  struct CondJump : Instruction {
    Temp cond;
    Label label;

    CondJump(Temp cond, Label label) : cond(cond), label(label) {}
    virtual Operation op() { return Operation::CondJump; }
    virtual string toString() { return "cjmp " + cond.toString() + ", " + label.toString(); }
  };

  typedef vector<Instruction*> InstructionList;
}
