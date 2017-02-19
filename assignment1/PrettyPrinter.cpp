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

bool PrettyPrinter::maybeExtraIndent() {
  if (last_printed == '(' || last_printed == '[') {
    this->nextline();
    this->indent();
    return true;
  }
  return false;
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

bool PrettyPrinter::open(string name) {
  bool r = this->maybeExtraIndent();
  this->println(name);
  this->indent();
  return r;
}

void PrettyPrinter::close(bool extra) {
  this->dedent();
  if (extra)
    this->dedent();
  this->nextline();
}

bool PrettyPrinter::start(string name) {
  bool r = this->maybeExtraIndent();
  this->println(name + " {");
  this->indent();
  return r;
}

void PrettyPrinter::end(bool extra) {
  this->dedent();
  if (extra)
    this->dedent();
  this->println("}");
}

void PrettyPrinter::visit(Program& prog) {
  this->println("Program");
  this->println("-------");
  this->indent();
  prog.block->accept(*this);
}

void PrettyPrinter::visit(Block& block) {
  bool r = this->start("Block");
  for (Statement *stmt : block.statements) {
    stmt->accept(*this);
  }
  this->end(r);
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
  bool r = this->open("Assignment");
  assign.lhs->accept(*this);
  this->print(" = ");
  assign.expr->accept(*this);
  this->print(";");
  this->close(r);
}

void PrettyPrinter::visit(Call& call) {
  bool r1 = this->maybeExtraIndent();
  this->print("Call[");
  call.target->accept(*this);
  bool r2 = this->open("]");

  bool first = true;
  for (Expression *expr : call.arguments) {
    if (first)
      first = false;
    else
      this->nextline();
    expr->accept(*this);
  }
  this->close(r2);
  if (r1)
    this->dedent();
}

void PrettyPrinter::visit(CallStatement& cs) {
  cs.call->accept(*this);
}

void PrettyPrinter::visit(Global& global) {
  bool r = this->open("Global");
  this->print(global.name);
  this->close(r);
}

void PrettyPrinter::visit(IfStatement& is) {
  bool r1 = this->open("IfStatement");
  this->print("If[");
  is.cond->accept(*this);
  bool r2 = this->start("]");
  is.thenBlock->accept(*this);
  this->end(r2);
  if (!is.elseBlock->empty()) {
    r2 = this->start("Else");
    is.elseBlock->accept(*this);
    this->end(r2);
  }
  this->close(r1);
}

void PrettyPrinter::visit(WhileLoop& wl) {
  bool r1 = this->open("WhileLoop");
  this->print("While[");
  wl.cond->accept(*this);
  bool r2 = this->start("]");
  wl.body->accept(*this);
  this->end(r2);
  this->close(r1);
}

void PrettyPrinter::visit(Return& ret) {
  bool r = this->open("Return");
  ret.expr->accept(*this);
  this->close(r);
}

void PrettyPrinter::visit(Function& func) {
  this->print("Function(");
  bool first = true;
  for (Name *name : func.arguments) {
    if (first)
      first = false;
    else
      this->print(", ");
    name->accept(*this);
  }
  bool r = this->open(")");
  func.body->accept(*this);
  this->close(r);
}

void PrettyPrinter::visit(Record& rec) {
  bool r = this->open("Record");
  bool first = true;
  for (auto& kv : rec.record) {
    if (first)
      first = false;
    else
      this->nextline();
    this->print(kv.first);
    this->print(" : ");
    kv.second->accept(*this);
  }
  this->close(r);
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
