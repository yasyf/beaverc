#include <iostream>
#include <string>
#include "PrettyPrinter.h"

using namespace std;

void PrettyPrinter::print(string msg, bool newline = false) {
  if (!line_started) {
    line_started = true;
    for (int i = 0; i < this->indent_level; i++)
      cout << "  ";
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

void PrettyPrinter::open(string name) {
  this->println(name);
  this->indent();
}

void PrettyPrinter::close() {
  this->dedent();
  if (line_started)
    this->nextline();
}

void PrettyPrinter::start(string name) {
  this->println(name + " {");
  this->indent();
}

void PrettyPrinter::end() {
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
  this->start("Block");
  for (Statement *stmt : block.statements) {
    stmt->accept(*this);
  }
  this->end();
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
  this->open("Assignment");
  assign.lhs->accept(*this);
  this->print(" = ");
  assign.expr->accept(*this);
  this->print(";");
  this->close();
}

void PrettyPrinter::visit(Call& call) {
  this->print("Call[");
  call.target->accept(*this);
  this->open("]");

  bool first = true;
  for (Expression *expr : call.arguments) {
    if (first)
      first = false;
    else
      this->nextline();
    expr->accept(*this);
  }
  this->close();
}

void PrettyPrinter::visit(CallStatement& cs) {
  cs.call->accept(*this);
}

void PrettyPrinter::visit(Global& global) {
  this->open("Global");
  this->print(global.name);
  this->close();
}

void PrettyPrinter::visit(IfStatement& is) {
  this->open("IfStatement");
  this->print("If[");
  is.cond->accept(*this);
  this->start("]");
  is.thenBlock->accept(*this);
  this->end();
  if (!is.elseBlock->empty()) {
    this->start("Else");
    is.elseBlock->accept(*this);
    this->end();
  }
  this->close();
}

void PrettyPrinter::visit(WhileLoop& wl) {
  this->open("WhileLoop");
  this->print("While[");
  wl.cond->accept(*this);
  this->start("]");
  wl.body->accept(*this);
  this->end();
  this->close();
}

void PrettyPrinter::visit(Return& ret) {
  this->open("Return");
  ret.expr->accept(*this);
  this->close();
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
  this->open(")");
  func.body->accept(*this);
  this->close();
}

void PrettyPrinter::visit(Record& rec) {
  this->open("Record");
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
  this->close();
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
