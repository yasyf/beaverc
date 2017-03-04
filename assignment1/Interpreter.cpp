#include <iostream>
#include "Interpreter.h"

using namespace std;

Interpreter::Interpreter() : heap() {}

void Interpreter::print(string msg, bool newline = false) {
  cout << msg;
  if (newline)
    cout << endl;
}

void Interpreter::println(string msg) {
  print(msg, true);
}

void Interpreter::exec(AST_node *node, Value *asval = nullptr) {
  asvals.push(asval);
  node->accept(*this);
  asvals.pop();
}

Value* Interpreter::eval(Expression *exp) {
  exec(exp);
  return retval;
}

void Interpreter::assign(LHS *lhs, Value *asval) {
  exec(lhs, asval);
}

void Interpreter::visit(Block& block) {
  for (Statement *stmt : block.statements) {
    exec(stmt);
  }
}

void Interpreter::visit(Assignment& assign) {
  Value *asval = eval(assign.expr);
  assign(assign.lhs, asval);
}

void Interpreter::visit(Name& name) {
  if (asvals.top()) {
    heap.UpdateVar(name.name, asvals.top());
  } else {
    retval = heap.ReadVar(name.name);
  }
}

void Interpreter::visit(FieldDereference& fd) {
  Value *base = eval(fd.base);
  RecordValue *record;
  if (!(record = dynamic_cast<RecordValue*>(base))) {
    throw IllegalCastException(base);
  }
  if (asvals.top()) {
    record.Update(fd.field.name, asvals.top());
  } else {
    retval = record.Read(fd.field.name);
  }
}

void Interpreter::visit(IndexExpression& ie) {
  Value *base = eval(ie.base);
  RecordValue *record;
  if (!(record = dynamic_cast<RecordValue*>(base))) {
    throw IllegalCastException(base);
  }
  string field = eval(ie.index).toString();
  if (asvals.top()) {
    record.Update(field, asvals.top());
  } else {
    retval = record.Read(field);
  }
}
