#include <iostream>
#include <string>
#include <algorithm>
#include "Transpiler.h"

using namespace std;

namespace BC {
  void Transpiler::visit(AST::Program& prog) {
    prog.block->accept(*this);
  }

  void Transpiler::visit(AST::Block& block) {
  }

  void Transpiler::visit(AST::Name& name) {
  }

  void Transpiler::visit(AST::IndexExpression& ie) {
  }

  void Transpiler::visit(AST::FieldDereference& fd) {
  }

  void Transpiler::visit(AST::Assignment& assign) {
  }

  void Transpiler::visit(AST::Call& call) {
  }

  void Transpiler::visit(AST::CallStatement& cs) {
  }

  void Transpiler::visit(AST::Global& global) {
  }

  void Transpiler::visit(AST::IfStatement& is) {
  }

  void Transpiler::visit(AST::WhileLoop& wl) {
  }

  void Transpiler::visit(AST::Return& ret) {
  }

  void Transpiler::visit(AST::Function& func) {
  }

  void Transpiler::visit(AST::Record& rec) {
  }

  void Transpiler::visit(AST::ValueConstant<bool>& boolconst) {
  }

  void Transpiler::visit(AST::StringConstant& strconst) {
  }

  void Transpiler::visit(AST::ValueConstant<int>& intconst) {
  }

  void Transpiler::visit(AST::NullConstant& nullconst) {
  }

  template <BinOpSym op>
  void Transpiler::visitBinop(AST::BinaryOp<op>& binop, string opstring) {
  }

  template <UnOpSym op>
  void Transpiler::visitUnop(AST::UnaryOp<op>& unop, string opstring) {
  }

  void Transpiler::visit(AST::BinaryOp<OR>& orop) {
    this->visitBinop(orop, "|");
  }

  void Transpiler::visit(AST::BinaryOp<AND>& andop) {
    this->visitBinop(andop, "&");
  }

  void Transpiler::visit(AST::BinaryOp<LT>& ltop) {
    this->visitBinop(ltop, "<");
  }

  void Transpiler::visit(AST::BinaryOp<LTE>& lteop) {
    this->visitBinop(lteop, "<=");
  }

  void Transpiler::visit(AST::BinaryOp<GT>& gtop) {
    this->visitBinop(gtop, ">");
  }

  void Transpiler::visit(AST::BinaryOp<GTE>& gteop) {
    this->visitBinop(gteop, ">=");
  }

  void Transpiler::visit(AST::BinaryOp<EQ>& eqop) {
    this->visitBinop(eqop, "==");
  }

  void Transpiler::visit(AST::BinaryOp<PLUS>& plusop) {
    this->visitBinop(plusop, "+");
  }

  void Transpiler::visit(AST::BinaryOp<MINUS>& minusop) {
    this->visitBinop(minusop, "-");
  }

  void Transpiler::visit(AST::BinaryOp<MUL>& mulop) {
    this->visitBinop(mulop, "*");
  }

  void Transpiler::visit(AST::BinaryOp<DIV>& divop) {
    this->visitBinop(divop, "/");
  }

  void Transpiler::visit(AST::UnaryOp<NOT>& notop) {
    this->visitUnop(notop, "!");
  }

  void Transpiler::visit(AST::UnaryOp<NEG>& negop) {
    this->visitUnop(negop, "-");
  }
}
