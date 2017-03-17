#pragma once
#include <string>
#include "AST.fwd.h"

//You will need to a virtual visitor class with a
//visit method for each different type of expression and statement
//as defined in AST.h

class Visitor {
public:
  // For each AST node, you need a virtual method of the form
  virtual void visit(Program& prog) = 0;
  virtual void visit(Block& block) = 0;
  virtual void visit(Name& name) = 0;
  virtual void visit(IndexExpression& ie) = 0;
  virtual void visit(FieldDereference& fd) = 0;
  virtual void visit(Assignment& assign) = 0;
  virtual void visit(Call& call) = 0;
  virtual void visit(CallStatement& cs) = 0;
  virtual void visit(Global& global) = 0;
  virtual void visit(IfStatement& is) = 0;
  virtual void visit(WhileLoop& wl) = 0;
  virtual void visit(Return& ret) = 0;
  virtual void visit(Function& func) = 0;
  virtual void visit(Record& rec) = 0;
  virtual void visit(ValueConstant<bool>& boolconst) = 0;
  virtual void visit(StringConstant& strconst) = 0;
  virtual void visit(ValueConstant<int>& intconst) = 0;
  virtual void visit(NullConstant& nullconst) = 0;
  virtual void visit(BinaryOp<OR>& orop) = 0;
  virtual void visit(BinaryOp<AND>& andop) = 0;
  virtual void visit(BinaryOp<LT>& ltop) = 0;
  virtual void visit(BinaryOp<LTE>& lteop) = 0;
  virtual void visit(BinaryOp<GT>& gtop) = 0;
  virtual void visit(BinaryOp<GTE>& gteop) = 0;
  virtual void visit(BinaryOp<EQ>& eqop) = 0;
  virtual void visit(BinaryOp<PLUS>& plusop) = 0;
  virtual void visit(BinaryOp<MINUS>& minusop) = 0;
  virtual void visit(BinaryOp<MUL>& mulop) = 0;
  virtual void visit(BinaryOp<DIV>& divop) = 0;
  virtual void visit(UnaryOp<NOT>& notop) = 0;
  virtual void visit(UnaryOp<NEG>& negop) = 0;
};


