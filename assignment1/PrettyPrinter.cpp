#include <iostream>
#include <string>
#include "PrettyPrinter.h"

using namespace std;

void PrettyPrinter::print(string msg, bool newline = false) {
  if (!line_started) {
    line_started = true;
    for (int i = 0; i < this->indent_level; i++)
      cout << " ";
  }
  cout << msg;
  if (newline)
    this->nextline();
}

void PrettyPrinter::println(string msg) {
  this->print(msg, true);
}

void PrettyPrinter::nextline() {
  line_started = false;
  cout << endl;
}

void PrettyPrinter::indent() {
  this->indent_level++;
}

void PrettyPrinter::dedent() {
  this->indent_level--;
}

void PrettyPrinter::visit(Program& prog) {
  this->println("Program");
  this->indent();
  prog.block->accept(*this);
}

void PrettyPrinter::visit(Block& block) {
  this->println("{");
  this->indent();
  for (Statement *stmt : block.statements) {
    stmt->accept(*this);
  }
  this->dedent();
  this->println("}");
}

void PrettyPrinter::visit(Name& name) {
  this->print(name.name);
}

void PrettyPrinter::visit(IndexExpression& ie) {}

void PrettyPrinter::visit(FieldDereference& fd) {}

void PrettyPrinter::visit(Assignment& assign) {
  assign.lhs->accept(*this);
  this->print(" = ");
  assign.expr->accept(*this);
  this->nextline();
}

void PrettyPrinter::visit(Call& call) {}

void PrettyPrinter::visit(CallStatement& cs) {}

void PrettyPrinter::visit(Global& global) {}

void PrettyPrinter::visit(IfStatement& is) {}

void PrettyPrinter::visit(WhileLoop& wl) {}

void PrettyPrinter::visit(Return& ret) {}

void PrettyPrinter::visit(Function& func) {}

void PrettyPrinter::visit(Record& rec) {}

void PrettyPrinter::visit(ValueConstant<bool>& boolconst) {
  this->print(boolconst.value ? "true" : "false");
}

void PrettyPrinter::visit(ValueConstant<std::string>& strconst) {
  this->print(strconst.value);
}

void PrettyPrinter::visit(ValueConstant<int>& intconst) {
  this->print(to_string(intconst.value));
}

void PrettyPrinter::visit(NullConstant& nullconst) {
  this->print("None");
}

template <BinOpSym op>
void PrettyPrinter::visitBinop(BinaryOp<op>& binop, string opstring) {
  this->print("(");
  binop.left->accept(*this);
  this->print(")");
  this->print(" " + opstring + " ");
  this->print("(");
  binop.right->accept(*this);
  this->print(")");
}

template <UnOpSym op>
void PrettyPrinter::visitUnop(UnaryOp<op>& unop, string opstring) {
  this->print(opstring);
  this->print("(");
  unop.expr->accept(*this);
  this->print(")");
}

void PrettyPrinter::visit(BinaryOp<OR>& orop) {
  this->visitBinop(orop, "|");
}

void PrettyPrinter::visit(BinaryOp<AND>& andop) {
  this->visitBinop(andop, "&");
}

void PrettyPrinter::visit(BinaryOp<LT>& ltop) {
  this->visitBinop(ltop, "<");
}

void PrettyPrinter::visit(BinaryOp<LTE>& lteop) {
  this->visitBinop(lteop, "<=");
}

void PrettyPrinter::visit(BinaryOp<GT>& gtop) {
  this->visitBinop(gtop, ">");
}

void PrettyPrinter::visit(BinaryOp<GTE>& gteop) {
  this->visitBinop(gteop, ">=");
}

void PrettyPrinter::visit(BinaryOp<EQ>& eqop) {
  this->visitBinop(eqop, "==");
}

void PrettyPrinter::visit(BinaryOp<PLUS>& plusop) {
  this->visitBinop(plusop, "+");
}

void PrettyPrinter::visit(BinaryOp<MINUS>& minusop) {
  this->visitBinop(minusop, "-");
}

void PrettyPrinter::visit(BinaryOp<MUL>& mulop) {
  this->visitBinop(mulop, "*");
}

void PrettyPrinter::visit(BinaryOp<DIV>& divop) {
  this->visitBinop(divop, "/");
}

void PrettyPrinter::visit(UnaryOp<NOT>& notop) {
  this->visitUnop(notop, "!");
}

void PrettyPrinter::visit(UnaryOp<NEG>& negop) {
  this->visitUnop(negop, "-");
}
