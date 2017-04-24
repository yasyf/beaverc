#pragma once

#include "../bccompiler/Types.h"

using namespace std;

namespace IR {
  struct Operand {
    virtual string toString() = 0;
  };

  struct Reg : Operand {
    size_t num;

    Reg(size_t num) : num(num) {}
    virtual string toString() { return "%" + to_string(num); }
  };

  struct Var : Operand {
    string name;

    Var(string name) : name(name) {}
    virtual string toString() { return "%" + name; }
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

  // TODO: Is this necessary?
  enum class Operation {
    AllocVar,
    StoreVar,
    StoreReg,
    Add,
    Return,
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
  struct StoreVar : Instruction {
    Var dest;
    S src;

    StoreVar(Var dest, S src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::StoreVar; }
    virtual string toString() { return dest.toString() + " = " + src.toString(); }
  };

  template<typename S>
  struct StoreReg : Instruction {
    Reg dest;
    S src;

    StoreReg(Reg dest, S src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::StoreReg; }
    virtual string toString() { return dest.toString() + " = " + src.toString(); }
  };

  struct Add : Instruction {
    Reg dest;
    Reg src1;
    Reg src2;

    Add(Reg dest, Reg src1, Reg src2) : dest(dest), src1(src1), src2(src2) {}
    virtual Operation op() { return Operation::Add; }
    virtual string toString() { return dest.toString() + " = " + src1.toString() + " + " + src2.toString(); }
  };

  struct Return : Instruction {
    Reg val;

    Return(Reg val) : val(val) {}
    virtual Operation op() { return Operation::Return; }
    virtual string toString() { return "return " + val.toString(); }
  };

  typedef vector<Instruction*> InstructionList;
}
