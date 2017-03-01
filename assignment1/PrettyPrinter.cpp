#include <iostream>
#include <string>
#include <algorithm>
#include "PrettyPrinter.h"

using namespace std;

void PrettyPrinter::print(string msg, bool newline = false) {
  if (!line_started) {
    line_started = true;
    for (int i = 0; i < this->indent_level; i++)
      cout << "  ";
  }

  last_printed = msg.back();

  cout << msg;
  if (newline)
    this->nextline();
}

void PrettyPrinter::println(string msg) {
  this->print(msg, true);
}

void PrettyPrinter::nextline() {
  if (!line_started)
    return;
  line_started = false;
  last_printed = 0;
  cout << endl;
}

void PrettyPrinter::indent() {
  this->indent_level++;
}

void PrettyPrinter::dedent() {
  this->indent_level--;
}

void PrettyPrinter::visit(Program& prog) {
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

void PrettyPrinter::visit(IndexExpression& ie) {
  ie.base->accept(*this);
  this->print("[");
  ie.index->accept(*this);
  this->print("]");
}

void PrettyPrinter::visit(FieldDereference& fd) {
  fd.base->accept(*this);
  this->print(".");
  fd.field->accept(*this);
}

void PrettyPrinter::visit(Assignment& assign) {
  assign.lhs->accept(*this);
  this->print(" = ");
  assign.expr->accept(*this);
  this->println(";");
}

void PrettyPrinter::visit(Call& call) {
  call.target->accept(*this);
  this->print("(");

  bool first = true;
  for (Expression *expr : call.arguments) {
    if (first)
      first = false;
    else
      this->print(", ");
    expr->accept(*this);
  }
  this->print(")");
}

void PrettyPrinter::visit(CallStatement& cs) {
  cs.call->accept(*this);
  this->println(";");
}

void PrettyPrinter::visit(Global& global) {
  this->print("global ");
  this->print(global.name);
  this->println(";");
}

void PrettyPrinter::visit(IfStatement& is) {
  this->print("if (");
  is.cond->accept(*this);
  this->print(") ");
  is.thenBlock->accept(*this);
  if (!is.elseBlock->empty()) {
    this->print("else ");
    is.elseBlock->accept(*this);
  }
}

void PrettyPrinter::visit(WhileLoop& wl) {
  this->print("while (");
  wl.cond->accept(*this);
  this->print(") ");
  wl.body->accept(*this);
}

void PrettyPrinter::visit(Return& ret) {
  this->print("return ");
  ret.expr->accept(*this);
  this->println(";");
}

void PrettyPrinter::visit(Function& func) {
  this->print("fun(");
  bool first = true;
  for (Name *name : func.arguments) {
    if (first)
      first = false;
    else
      this->print(" ");
    name->accept(*this);
  }
  this->println(") {");
  this->indent();
  func.body->accept(*this);
  this->dedent();
  this->println("}");
}

void PrettyPrinter::visit(Record& rec) {
  this->print("{");
  bool first = true;
  for (auto& kv : rec.record) {
    if (first)
      first = false;
    else
      this->print("; ");
    this->print(kv.first);
    this->print(" : ");
    kv.second->accept(*this);
  }
  this->println("}");
}

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
