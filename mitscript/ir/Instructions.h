#pragma once
#include "include/x64asm.h"
#include "../bccompiler/Types.h"
#include "../vm/Value.h"
#include <experimental/optional>
#include <vector>
#include <cassert>
#include <climits>
#include <sstream>

using namespace std;
using namespace std::experimental;

namespace IR {
  const int CONST_SRC_HINT    = (1 << 0);
  const int VAR_SRC_HINT      = (1 << 1);

  const int INT_TYPE_HINT    = (1 << 2);
  const int BOOL_TYPE_HINT   = (1 << 3);
  const int STRING_TYPE_HINT = (1 << 4);
  const int NONE_TYPE_HINT   = (1 << 5);

  const int SRC_HINT_MASK = CONST_SRC_HINT | VAR_SRC_HINT;

  struct Operand {
    int live_start = -1;
    int live_end = INT_MAX;
    int type_hint = 0;
    uint64_t src_val = 0;
    optional<x64asm::R64> reg;

    virtual string toString() const = 0;

     virtual void transferHint(shared_ptr<Operand> op) {
      this->type_hint |= op->type_hint & ~SRC_HINT_MASK;
    }

    bool hasHint() {
      return type_hint != 0;
    }

    void hintConst(uint64_t val) {
      this->src_val = val;
      hint(CONST_SRC_HINT);
    }

    bool isConst() {
      return canBe(CONST_SRC_HINT);
    }

    VM::Value getConst() {
      assert(isConst());
      return VM::Value(src_val);
    }

    void hintVar(uint64_t num) {
      this->src_val = num;
      hint(VAR_SRC_HINT);
    }

    bool isVar() {
      return canBe(VAR_SRC_HINT);
    }

    uint64_t getVar() {
      assert(isVar());
      return src_val;
    }

    void hintInt() {
      hint(INT_TYPE_HINT);
    }

    bool canBeInt() {
      return canBe(INT_TYPE_HINT);
    }

    bool isInt() {
      return is(INT_TYPE_HINT);
    }

    void hintBool() {
      hint(BOOL_TYPE_HINT);
    }

    bool canBeBool() {
      return canBe(BOOL_TYPE_HINT);
    }

    bool isBool() {
      return is(BOOL_TYPE_HINT);
    }

    void hintString() {
      hint(STRING_TYPE_HINT);
    }

    bool canBeString() {
      return canBe(STRING_TYPE_HINT);
    }

    bool isString() {
      return is(STRING_TYPE_HINT);
    }

    void hintNone() {
      hint(NONE_TYPE_HINT);
    }

    bool canBeNone() {
      return canBe(NONE_TYPE_HINT);
    }

    bool isNone() {
      return is(NONE_TYPE_HINT);
    }

  protected:
    bool canBe(int type_const) {
      return type_hint & type_const;
    }

    ostream& debugInfo(ostream& os) const {
      os << " (" << to_string(type_hint) << ")";
      os << " {" << to_string(live_start) << " - " << to_string(live_end) << "}";
      if (reg) {
        os << " [" << *reg << "]";
      }
      return os;
    }

  private:
    void hint(int type_const) {
      this->type_hint |= type_const;
    }

    bool is(int type_const) {
      return (type_hint & ~SRC_HINT_MASK) == type_const;
    }
  };

  struct Label : Operand {
    size_t num;

    Label(size_t num) : num(num) {}
    virtual string toString() const override { return "l" + to_string(num); }
  };

  struct Temp : Operand {
    size_t num;
    bool shared_reg = false;

    Temp(size_t num) : num(num) {}

    virtual void transferHint(shared_ptr<Operand> op) {
      this->src_val = op->src_val;
      this->type_hint = op->type_hint;
    }

    #ifdef DEBUG
      virtual string toString() const override {
        ostringstream os;
        os << "t" << to_string(num);
        debugInfo(os);
        return os.str();
      }
    #else
      virtual string toString() const override { return "t" + to_string(num); }
    #endif
  };

  struct RetVal : Operand {
    virtual string toString() const override { return "retval"; }
  };

  struct Var : Operand {
    size_t num;
    optional<x64asm::R64> last_reg;

    Var(size_t num) : num(num) {
      hintVar(num);
    }

    #ifdef DEBUG
      virtual string toString() const override {
        ostringstream os;
        os << "%" << to_string(num);
        debugInfo(os);
        return os.str();
      }
    #else
      virtual string toString() const override { return "%" + to_string(num); }
    #endif
  };

  struct Glob : Operand {
    size_t num;

    Glob(size_t num) : num(num) {}

    #ifdef DEBUG
      virtual string toString() const override { return "%%" + to_string(num) + " (" + to_string(type_hint) + ")"; }
    #else
      virtual string toString() const override { return "%%" + to_string(num); }
    #endif
  };

  struct Function : Operand {
    size_t num;

    Function(size_t num) : num(num) {}
    virtual string toString() const override { return "f" + to_string(num); }
  };

  struct Ref : Operand {
    size_t num;

    Ref(size_t num) : num(num) {}
    virtual string toString() const override { return "r" + to_string(num); }
  };

  struct Deref : Operand {
    size_t num;

    Deref(size_t num) : num(num) {}
    virtual string toString() const override { return "*r" + to_string(num); }
  };

  struct Const : Operand {
    uint64_t val;

    Const(VM::Value value) {
      this->val = value.value;
      if (value.isInteger())
        hintInt();
      else if (value.isString())
        hintString();
      else if (value.isBoolean())
        hintBool();
      else if (value.isNone())
        hintNone();
      hintConst(val);
    }

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
        char * cstr = new char [val->value.length()+1];
        strcpy(cstr, val->value.c_str());
        this->val = VM::Value::makeStringConstant(cstr).value;
        hintString();
      }
      else {
        throw "invalid Constant!";
      }
      hintConst(val);
    }

    virtual string toString() const override { return "$" + to_string(val); }
  };

  enum class Operation {
    Assign,
    ForceLoad,
    Store,
    Add,
    IntAdd,
    Sub,
    Mul,
    Div,
    Gt,
    Geq,
    Eq,
    FastEq,
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
    CallAssert,
    Noop,
    Fork,
  };

  enum class Helper {
    GarbageCollect,
    AllocRecord,
    FieldLoad,
    FieldStore,
    IndexLoad,
    IndexStore,
    ThrowUninitialized,
  };

  enum class Assert {
    AssertInt,
    AssertNotZero,
    AssertBool,
  };

  struct Instruction {
    virtual string toString() const = 0;
    virtual Operation op() = 0;
  };

  template<typename S>
  struct Assign : Instruction {
    shared_ptr<Temp> dest;
    shared_ptr<S> src;

    Assign(shared_ptr<Temp> dest, shared_ptr<S> src) : dest(dest), src(src) {
      dest->transferHint(src);
    }
    virtual Operation op() { return Operation::Assign; }
    virtual string toString() const override { return dest->toString() + " = " + src->toString(); }
  };

  template<typename S>
  struct ForceLoad : Instruction {
    shared_ptr<S> src;

    ForceLoad(shared_ptr<S> src) : src(src) {}
    virtual Operation op() { return Operation::ForceLoad; }
    virtual string toString() const override { return "force_load " + src->toString(); }
  };

  template<typename D>
  struct Store : Instruction {
    shared_ptr<D> dest;
    shared_ptr<Temp> src;

    Store(shared_ptr<D> dest, shared_ptr<Temp> src) : dest(dest), src(src) {
      dest->transferHint(src);
    }
    virtual Operation op() { return Operation::Store; }
    virtual string toString() const override { return dest->toString() + " = " + src->toString(); }
  };

  template<Operation Op>
  struct BinOp : Instruction {
    shared_ptr<Temp> dest;
    shared_ptr<Temp> src1;
    shared_ptr<Temp> src2;

    BinOp(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : dest(dest), src1(src1), src2(src2) {}
    virtual Operation op() { return Op; }
    virtual string opString() const = 0;
    virtual string toString() const override { return dest->toString() + " = " + src2->toString() + " " + opString() + " " + src1->toString(); }
  };

  struct Add : BinOp<Operation::Add> {
    using BinOp::BinOp;
    virtual string opString() const override { return "+"; }
  };

  struct IntAdd : public Add {
    IntAdd(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : Add(dest, src1, src2) {
      dest->hintInt();
    }
    virtual Operation op() { return Operation::IntAdd; }
  };

  struct Sub : BinOp<Operation::Sub> {
    Sub(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintInt();
    }
    virtual string opString() const override { return "-"; }
  };

  struct Mul : BinOp<Operation::Mul> {
    Mul(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintInt();
    }
    virtual string opString() const override { return "*"; }
  };

  struct Div : BinOp<Operation::Div> {
    Div(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintInt();
    }
    virtual string opString() const override { return "/"; }
  };

  struct Gt : BinOp<Operation::Gt> {
    Gt(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintBool();
    }
    virtual string opString() const override { return "<"; }
  };

  struct Geq : BinOp<Operation::Geq> {
    Geq(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintBool();
    }
    virtual string opString() const override { return "<="; }
  };

  struct Eq : BinOp<Operation::Eq> {
    Eq(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintBool();
    }
    virtual string opString() const override { return "=="; }
  };

  struct FastEq : public Eq {
    using Eq::Eq;
    virtual Operation op() { return Operation::FastEq; }
  };

  struct And : BinOp<Operation::And> {
    And(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintBool();
    }
    virtual string opString() const override { return "&"; }
  };

  struct Or : BinOp<Operation::Or> {
    Or(shared_ptr<Temp> dest, shared_ptr<Temp> src1, shared_ptr<Temp> src2) : BinOp(dest, src1, src2) {
      dest->hintBool();
    }
    virtual string opString() const override { return "|"; }
  };

  template<Operation Op>
  struct UnOp : Instruction {
    shared_ptr<Temp> dest;
    shared_ptr<Temp> src;

    UnOp(shared_ptr<Temp> dest, shared_ptr<Temp> src) : dest(dest), src(src) {
      addHint(dest);
    }
    virtual void addHint(shared_ptr<Temp> dest) {};
    virtual Operation op() { return Op; }
    virtual string opString() const = 0;
    virtual string toString() const override { return dest->toString() + " = " + opString() + src->toString(); }
  };

  struct Neg : UnOp<Operation::Neg> {
    Neg(shared_ptr<Temp> dest, shared_ptr<Temp> src) : UnOp(dest, src) {
      dest->hintInt();
    }
    virtual string opString() const override { return "-"; }
  };

  struct Not : UnOp<Operation::Not> {
    Not(shared_ptr<Temp> dest, shared_ptr<Temp> src) : UnOp(dest, src) {
      dest->hintBool();
    }
    virtual string opString() const override { return "!"; }
  };

  struct Call : Instruction {
    shared_ptr<Temp> closure;
    vector<shared_ptr<Temp>> args;

    Call(shared_ptr<Temp> closure, vector<shared_ptr<Temp>> args) : closure(closure), args(args) {}
    virtual Operation op() { return Operation::Call; }
    virtual string toString() const override { return "call " + closure->toString(); }
  };

  struct AllocClosure : Instruction {
    shared_ptr<Temp> function;
    vector<shared_ptr<Temp>> refs;

    AllocClosure(shared_ptr<Temp> function, vector<shared_ptr<Temp>> refs) : function(function), refs(refs) {}
    virtual Operation op() { return Operation::AllocClosure; }
    #ifdef DEBUG
      virtual string toString() const override {
        string result = "alloc_closure " + function->toString() + " -- ";
        for (auto t : refs) {
          result += t->toString();
        }
        return result;
      }
    #else
      virtual string toString() const override { return "alloc_closure"; }
    #endif
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
    virtual string toString() const override { return "call_helper " + to_string(static_cast<int>(H)); }
  };

  template<Assert A>
  struct CallAssert : Instruction {
    shared_ptr<Temp> arg;

    CallAssert(shared_ptr<Temp> arg) : arg(arg) {}

    static Assert assert_() { return A; }
    virtual Operation op() { return Operation::CallAssert; }
    virtual string toString() const override { return "call_assert_" + to_string(static_cast<int>(A)) + " " + arg->toString(); }
  };


  struct Return : Instruction {
    shared_ptr<Temp> val;

    Return(shared_ptr<Temp> val) : val(val) {}
    virtual Operation op() { return Operation::Return; }
    virtual string toString() const override { return "return " + val->toString(); }
  };

  struct OutputLabel : Instruction {
    shared_ptr<Label> label;

    OutputLabel(shared_ptr<Label> label) : label(label) {}
    virtual Operation op() { return Operation::OutputLabel; }
    virtual string toString() const override { return label->toString() + ":"; }
  };

  struct Jump : Instruction {
    shared_ptr<Label> label;

    Jump(shared_ptr<Label> label) : label(label) {}
    virtual Operation op() { return Operation::Jump; }
    virtual string toString() const override { return "jmp " + label->toString(); }
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
    virtual string toString() const override { return "cjmp " + cond->toString() + ", " + label->toString(); }
  };

  struct Noop : Instruction {
    virtual Operation op() { return Operation::Noop; }
    virtual string toString() const override { return "noop"; }

    static Noop* Singleton() {
      static Noop* instance = nullptr;

      if (!instance)
        instance = new Noop();

      return instance;
    }
  };

  struct Fork : Instruction {
    shared_ptr<Temp> src;
    shared_ptr<Temp> dest1;
    shared_ptr<Temp> dest2;

    Fork(shared_ptr<Temp> src, shared_ptr<Temp> dest1, shared_ptr<Temp> dest2) : src(src), dest1(dest1), dest2(dest2) {
      dest1->transferHint(src);
      dest2->transferHint(src);
    }
    virtual Operation op() { return Operation::Fork; }
    virtual string toString() const override { return src->toString() + " -> " + dest1->toString() + ", " + dest2->toString(); }
  };

  typedef vector<Instruction*> InstructionList;
}

inline ostream& operator<<(ostream& os, const IR::Operand& op) {
  os << op.toString();
  return os;
}

inline ostream& operator<<(ostream& os, const IR::Instruction& op) {
  os << op.toString();
  return os;
}
