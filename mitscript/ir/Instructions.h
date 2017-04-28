#pragma once

#include "../bccompiler/Types.h"

using namespace std;

namespace IR {
  struct Label {
    string name;

    Label(string name) : name(name) {}
  };

  struct Operand {
    virtual string toString() = 0;
  };

  struct Temp : Operand {
    size_t num;

    Temp(size_t num) : num(num) {}
    virtual string toString() { return "t" + to_string(num); }
  };

  struct Ret : Operand {};

  struct Var : Operand {
    string name;

    Var(string name) : name(name) {}
    virtual string toString() { return "%" + name; }
  };

  struct Glob : Operand {
    string name;

    Glob(string name) : name(name) {}
    virtual string toString() { return "%%" + name; }
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
    AssignTemp,
    AllocVar,
    StoreVar,
    StoreGlob,
    Add,
    Call,
    Return,
    OutputLabel,
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
  struct StoreGlob : Instruction {
    Glob dest;
    S src;

    StoreGlob(Glob dest, S src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::StoreGlob; }
    virtual string toString() { return dest.toString() + " = " + src.toString(); }
  };

  template<typename S>
  struct AssignTemp : Instruction {
    Temp dest;
    S src;

    AssignTemp(Temp dest, S src) : dest(dest), src(src) {}
    virtual Operation op() { return Operation::AssignTemp; }
    virtual string toString() { return dest.toString() + " = " + src.toString(); }
  };

  struct Add : Instruction {
    Temp dest;
    Temp src1;
    Temp src2;

    Add(Temp dest, Temp src1, Temp src2) : dest(dest), src1(src1), src2(src2) {}
    virtual Operation op() { return Operation::Add; }
    virtual string toString() { return dest.toString() + " = " + src1.toString() + " + " + src2.toString(); }
  };

  struct Call : Instruction {
    Label label;

    Call(Label label) : label(label) {}
    virtual Operation op() { return Operation::Call; }
    virtual string toString() { return "call " + label.name; }
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
    virtual string toString() { return label.name + ":"; }
  };

  typedef vector<Instruction*> InstructionList;
}