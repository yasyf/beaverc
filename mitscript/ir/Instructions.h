#pragma once

#include "../bccompiler/Types.h"
#include <vector>

using namespace std;

namespace IR {
  struct Operand {
    virtual string toString() = 0;
  };

  struct Label {
    string name;

    Label(string name) : name(name) {}
    virtual string toString() { return name; }
  };

  struct Temp : Operand {
    size_t num;

    Temp(size_t num) : num(num) {}
    virtual string toString() { return "t" + to_string(num); }
  };

  struct Ret : Operand {};

  struct Var : Operand {
    size_t num;

    Var(size_t num) : num(num) {}
    virtual string toString() { return "%" + to_string(num); }
  };

  struct Glob : Operand {
    string name;

    Glob(string name) : name(name) {}
    virtual string toString() { return "%%" + name; }
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
    int64_t val;

    Const(int64_t val) : val(val) {}

    Const(std::shared_ptr<BC::Constant> constant) {
      if (auto val = dynamic_pointer_cast<BC::Integer>(constant))
        this->val = val->value;
      else if (auto val = dynamic_pointer_cast<BC::None>(constant))
        this->val = 0;
      else if (auto val = dynamic_pointer_cast<BC::Boolean>(constant))
        this->val = val->value ? 1 : 0;
      else if (auto val = dynamic_pointer_cast<BC::String>(constant))
        this->val = 0; // TODO
      else
        throw "invalid Constant!";
    }

    virtual string toString() { return "$" + to_string(val); }
  };

  enum class Operation {
    Assign,
    Store,
    AllocVar,
    Add,
    Sub,
    Mul,
    Div,
    Gt,
    Geq,
    And,
    Or,
    Not,
    Neg,
    Call,
    Return,
    OutputLabel,
    CallHelper,
  };

  enum class Helper {
    AssertInt,
    AssertBool,
  };

  struct Instruction {
    virtual string toString() = 0;
    virtual Operation op() = 0;
  };

  struct AllocVar : Instruction {
    Var var;
    size_t size;

    AllocVar(Var var, size_t size) : var(var), size(size) {}
    virtual Operation op() { return Operation::AllocVar; }
    virtual string toString() { return var.toString() + " = alloc(" + to_string(size) + ")"; }
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

  template<Helper H>
  struct CallHelper : Instruction {
    Temp arg;

    CallHelper(Temp arg) : arg(arg) {}
    static Helper helper() { return H; }
    virtual Operation op() { return Operation::CallHelper; }
    virtual string toString() { return "call_helper " + to_string(static_cast<int>(H)); }
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

  typedef vector<Instruction*> InstructionList;
}
