#pragma once

#include "../bccompiler/Types.h"
#include "../vm/Value.h"
#include <vector>

using namespace std;

namespace IR {
  const int INT_TYPE_HINT    = (1 << 0);
  const int BOOL_TYPE_HINT   = (1 << 1);
  const int STRING_TYPE_HINT = (1 << 2);
  const int NONE_TYPE_HINT   = (1 << 3);

  struct Operand {
    int type_hint = 0;

    virtual string toString() = 0;

    void hintInt() {
      this->type_hint |= INT_TYPE_HINT;
    }

    bool canBeInt() {
      return type_hint & INT_TYPE_HINT;
    }

    bool isInt() {
      return type_hint == INT_TYPE_HINT;
    }

    void hintBool() {
      this->type_hint |= BOOL_TYPE_HINT;
    }

    bool canBeBool() {
      return type_hint & BOOL_TYPE_HINT;
    }

    bool isBool() {
      return type_hint == BOOL_TYPE_HINT;
    }

    void hintString() {
      this->type_hint |= STRING_TYPE_HINT;
    }

    bool canBeString() {
      return type_hint & STRING_TYPE_HINT;
    }

    bool isString() {
      return type_hint == STRING_TYPE_HINT;
    }

    void hintNone() {
      this->type_hint |= NONE_TYPE_HINT;
    }

    bool canBeNone() {
      return type_hint & NONE_TYPE_HINT;
    }

    bool isNone() {
      return type_hint == NONE_TYPE_HINT;
    }
  };

  struct Label {
    size_t num;

    Label(size_t num) : num(num) {}
    virtual string toString() { return "l" + to_string(num); }
  };

  struct Temp : Operand {
    size_t num;

    Temp(size_t num) : num(num) {}

    #ifdef DEBUG
      virtual string toString() { return "t" + to_string(num) + " (" + to_string(type_hint) + ")"; }
    #else
      virtual string toString() { return "t" + to_string(num); }
    #endif
  };

  struct RetVal : Operand {
    virtual string toString() { return "retval"; }
  };

  struct Var : Operand {
    size_t num;

    Var(size_t num) : num(num) {}

    #ifdef DEBUG
      virtual string toString() { return "%" + to_string(num) + " (" + to_string(type_hint) + ")"; }
    #else
      virtual string toString() { return "%" + to_string(num); }
    #endif
  };

  struct Glob : Operand {
    size_t num;

    Glob(size_t num) : num(num) {}

    #ifdef DEBUG
      virtual string toString() { return "%%" + to_string(num) + " (" + to_string(type_hint) + ")"; }
    #else
      virtual string toString() { return "%%" + to_string(num); }
    #endif
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

    Const(shared_ptr<BC::Constant> constant) {
      if (auto val = dynamic_pointer_cast<BC::Integer>(constant)) {
        this->val = VM::Value::makeInteger(val->value).value;
        hintInt();
      }
      else if (auto val = dynamic_pointer_cast<BC::None>(constant)) {
        this->val = VM::Value::makeNone().value;
        hintNone();
      }
      else if (auto val = dynamic_pointer_cast<BC::Boolean>(constant)) {
        this->val = VM::Value::makeBoolean(val->value).value;
        hintBool();
      }
      else if (auto val = dynamic_pointer_cast<BC::String>(constant)) {
        this->val = VM::Value::makeStringConstant(val->value.c_str()).value;
        hintString();
      }
      else {
        throw "invalid Constant!";
      }
    }

    virtual string toString() { return "$" + to_string(val); }
  };

  enum class Operation {
    Assign,
    Store,
    Add,
    IntAdd,
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
    ShortJump,
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
    shared_ptr<Temp> dest;
    shared_ptr<S> src;

    Assign(shared_ptr<Temp> dest, shared_ptr<S> src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::Assign; }
    virtual string toString() { return dest->toString() + " = " + src->toString(); }
  };

  template<typename D>
  struct Store : Instruction {
    shared_ptr<D> dest;
    shared_ptr<Temp> src;

    Store(shared_ptr<D> dest, shared_ptr<Temp> src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::Store; }
    virtual string toString() { return dest->toString() + " = " + src->toString(); }
  };

  template<Operation Op>
  struct BinOp : Instruction {
    shared_ptr<Temp> dest;
    shared_ptr<Temp> src1;
    shared_ptr<Temp> src2;

    BinOp(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : dest(dest), src1(src1), src2(src2) {}
    virtual Operation op() { return Op; }
    virtual string opString() = 0;
    virtual string toString() { return dest->toString() + " = " + src1->toString() + " " + opString() + " " + src2->toString(); }
  };

  struct Add : BinOp<Operation::Add> {
    using BinOp::BinOp;
    virtual string opString() { return "+"; }
  };

  struct IntAdd : public Add {
    using Add::Add;
    virtual Operation op() { return Operation::IntAdd; }
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
    shared_ptr<Temp> dest;
    shared_ptr<Temp> src;

    UnOp(shared_ptr<Temp> dest, shared_ptr<Temp> src) : dest(dest), src(src) {}
    virtual Operation op() { return Op; }
    virtual string opString() = 0;
    virtual string toString() { return dest->toString() + " = " + opString() + src->toString(); }
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
    shared_ptr<Temp> closure;
    vector<shared_ptr<Temp>> args;

    Call(shared_ptr<Temp> closure, vector<shared_ptr<Temp>> args) : closure(closure), args(args) {}
    virtual Operation op() { return Operation::Call; }
    virtual string toString() { return "call " + closure->toString(); }
  };

  struct AllocClosure : Instruction {
    shared_ptr<Temp> function;
    vector<shared_ptr<Temp>> refs;

    AllocClosure(shared_ptr<Temp> function, vector<shared_ptr<Temp>> refs) : function(function), refs(refs) {}
    virtual Operation op() { return Operation::AllocClosure; }
    virtual string toString() {
      string result = "alloc_closure " + function->toString() + " -- ";
      for (auto t : refs) {
        result += t->toString();
      }
      return result;
    }
  };

  template<Helper H>
  struct CallHelper : Instruction {
    size_t arg0;
    vector<shared_ptr<Temp>> args;

    CallHelper() {}

    CallHelper(shared_ptr<Temp> arg) {
      this->args = {arg};
    }

    CallHelper(size_t arg0) {
      this->arg0 = arg0;
    }

    CallHelper(size_t arg0, shared_ptr<Temp> arg) {
      this->arg0 = arg0;
      this->args = {arg};
    }

    CallHelper(shared_ptr<Temp> arg1, shared_ptr<Temp> arg2) {
      this->args = {arg1, arg2};
    }

    CallHelper(size_t arg0, shared_ptr<Temp> arg1, shared_ptr<Temp> arg2) {
      this->arg0 = arg0;
      this->args = {arg1, arg2};
    }

    CallHelper(shared_ptr<Temp> arg1, shared_ptr<Temp> arg2, shared_ptr<Temp> arg3) {
      this->args = {arg1, arg2, arg3};
    }

    static Helper helper() { return H; }
    virtual Operation op() { return Operation::CallHelper; }
    virtual string toString() { return "call_helper " + to_string(static_cast<int>(H)); }
  };


  struct Return : Instruction {
    shared_ptr<Temp> val;

    Return(shared_ptr<Temp> val) : val(val) {}
    virtual Operation op() { return Operation::Return; }
    virtual string toString() { return "return " + val->toString(); }
  };

  struct OutputLabel : Instruction {
    shared_ptr<Label> label;

    OutputLabel(shared_ptr<Label> label) : label(label) {}
    virtual Operation op() { return Operation::OutputLabel; }
    virtual string toString() { return label->toString() + ":"; }
  };

  struct Jump : Instruction {
    shared_ptr<Label> label;

    Jump(shared_ptr<Label> label) : label(label) {}
    virtual Operation op() { return Operation::Jump; }
    virtual string toString() { return "jmp " + label->toString(); }
  };

  struct ShortJump : public Jump {
    using Jump::Jump;
    virtual Operation op() { return Operation::ShortJump; }
  };

  struct CondJump : Instruction {
    shared_ptr<Temp> cond;
    shared_ptr<Label> label;

    CondJump(shared_ptr<Temp> cond, shared_ptr<Label> label) : cond(cond), label(label) {}
    virtual Operation op() { return Operation::CondJump; }
    virtual string toString() { return "cjmp " + cond->toString() + ", " + label->toString(); }
  };

  typedef vector<Instruction*> InstructionList;
}
