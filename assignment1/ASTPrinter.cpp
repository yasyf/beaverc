#include <iostream>
#include <string>
#include <algorithm>
#include "ASTPrinter.h"

using namespace std;

bool ASTPrinter::maybeExtraIndent() {
  if (last_printed == '(' || last_printed == '[') {
    this->nextline();
    this->indent();
    return true;
  }
  return false;
}

bool ASTPrinter::open(string name) {
  bool r = this->maybeExtraIndent();
  this->println(name);
  this->indent();
  return r;
}

void ASTPrinter::close(bool extra) {
  this->dedent();
  if (extra)
    this->dedent();
  this->nextline();
}

bool ASTPrinter::start(string name) {
  bool r = this->maybeExtraIndent();
  this->println(name + " {");
  this->indent();
  return r;
}

void ASTPrinter::end(bool extra) {
  this->dedent();
  if (extra)
    this->dedent();
  this->println("}");
}

void ASTPrinter::visit(Program& prog) {
  this->println("Program");
  this->println("-------");
  this->indent();
  prog.block->accept(*this);
}

void ASTPrinter::visit(Block& block) {
  bool r = this->start("Block");
  for (Statement *stmt : block.statements) {
    stmt->accept(*this);
  }
  this->end(r);
}

void ASTPrinter::visit(Assignment& assign) {
  bool r = this->open("Assignment");
  assign.lhs->accept(*this);
  this->print(" = ");
  assign.expr->accept(*this);
  this->print(";");
  this->close(r);
}

void ASTPrinter::visit(Call& call) {
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

void ASTPrinter::visit(Global& global) {
  bool r = this->open("Global");
  this->print(global.name);
  this->close(r);
}

void ASTPrinter::visit(IfStatement& is) {
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

void ASTPrinter::visit(WhileLoop& wl) {
  bool r1 = this->open("WhileLoop");
  this->print("While[");
  wl.cond->accept(*this);
  bool r2 = this->start("]");
  wl.body->accept(*this);
  this->end(r2);
  this->close(r1);
}

void ASTPrinter::visit(Return& ret) {
  bool r = this->open("Return");
  ret.expr->accept(*this);
  this->close(r);
}

void ASTPrinter::visit(Function& func) {
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

void ASTPrinter::visit(Record& rec) {
  bool r = this->open("Record");
  bool first = true;
  rec.record.iterate([this, &first] (string key, Expression *value) {
    if (first)
      first = false;
    else
      this->nextline();
    this->print(key);
    this->print(" : ");
    value->accept(*this);
  });
  this->close(r);
}
