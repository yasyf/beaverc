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

void Interpreter::eval(Expression *exp) {
  exp->accept(*this);
  return retval;
}

void Interpreter::visit(Block& block) {
  for (Statement *stmt : block.statements) {
    eval(stmt);
  }
}

void Interpreter::visit(Assignment& assign) {
  asval = eval(assign.expr);
  eval(assign.lhs);
  asval = nullptr;
}

void Interpreter::visit(Name& name) {
  if (asval) {
    heap.UpdateVar(name.name, asval);
  } else {
    retval = heap.ReadVar(name.name);
  }
}
