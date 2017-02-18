#pragma once

#include <iostream>
#include <string>
#include "Visitor.h"

//This is where you get to define your pretty printer class, which should be
//a subtype of visitor.
class PrettyPrinter : public Visitor {
  void visit(Program& prog) {
    cout << "Program" << endl;
    prog.block->accept(*this);
  }

  void visit(Block& block) {}

  void visit(Name& name) {}

  void visit(IndexExpression& ie) {}

  void visit(FieldDereference& fd) {}

  void visit(Assignment& assign) {}

  void visit(Call& call) {}

  void visit(CallStatement& cs) {}

  void visit(Global& global) {}

  void visit(IfStatement& is) {}

  void visit(WhileLoop& wl) {}

  void visit(Return& ret) {}

  void visit(Function& func) {}

  void visit(Record& rec) {}

  void visit(ValueConstant<bool>& boolconst) {}

  void visit(ValueConstant<std::string>& strconst) {}

  void visit(ValueConstant<int>& intconst) {}

  void visit(NullConstant& nullconst) {}

  void visit(BinaryOp<OR>& orop) {}

  void visit(BinaryOp<AND>& andop) {}

  void visit(BinaryOp<LT>& ltop) {}

  void visit(BinaryOp<LTE>& lteop) {}

  void visit(BinaryOp<GT>& gtop) {}

  void visit(BinaryOp<GTE>& gteop) {}

  void visit(BinaryOp<EQ>& eqop) {}

  void visit(BinaryOp<PLUS>& plusop) {}

  void visit(BinaryOp<MINUS>& minusop) {}

  void visit(BinaryOp<MUL>& mulop) {}

  void visit(BinaryOp<DIV>& divop) {}

  void visit(UnaryOp<NOT>& notop) {}

  void visit(UnaryOp<NEG>& negop) {}
};
