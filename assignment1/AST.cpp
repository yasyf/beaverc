#include "AST.h"
#include "Visitor.h"

using namespace std;

// Program

Program::Program() : block(new Block()) {}

void Program::accept(Visitor& v) {
  v.visit(*this);
}

// Block

Block::Block() : statements() {}

void Block::Append(Statement *statement) {
  this->statements.push_back(statement);
}

void Block::accept(Visitor& v) {
  v.visit(*this);
}

// Name

Name::Name(string name) : name(name) {}

void Name::accept(Visitor& v) {
  v.visit(*this);
}

// IndexExpression

IndexExpression::IndexExpression(LHS *base, Expression *index) : base(base), index(index) {}

void IndexExpression::accept(Visitor& v) {
  v.visit(*this);
}

// FieldDereference

FieldDereference::FieldDereference(LHS *base, Name *field) : base(base), field(field) {}

void FieldDereference::accept(Visitor& v) {
  v.visit(*this);
}

// Assignment

Assignment::Assignment(LHS *lhs, Expression *expr) : lhs(lhs), expr(expr) {}

void Assignment::accept(Visitor& v) {
  v.visit(*this);
}

// Call

Call::Call(LHS *target, vector<Expression *> arguments) : target(target), arguments(arguments) {}

void Call::accept(Visitor& v) {
  v.visit(*this);
}

// CallStatement

CallStatement::CallStatement(Call *call) : call(call) {}

void CallStatement::accept(Visitor& v) {
  v.visit(*this);
}

// Global

Global::Global(string name) : name(name) {}

void Global::accept(Visitor& v) {
  v.visit(*this);
}

// IfStatement

IfStatement::IfStatement(Expression *cond, Block *thenBlock, Block *elseBlock)
  : cond(cond), thenBlock(thenBlock), elseBlock(elseBlock)
{}

void IfStatement::accept(Visitor& v) {
  v.visit(*this);
}

// WhileLoop

WhileLoop::WhileLoop(Expression *cond, Block *body) : cond(cond), body(body) {}

void WhileLoop::accept(Visitor& v) {
  v.visit(*this);
}

// Return

Return::Return(Expression *expr) : expr(expr) {}

void Return::accept(Visitor& v) {
  v.visit(*this);
}

// Function

Function::Function(vector<Name *> arguments, Block *body) : arguments(arguments), body(body) {}

void Function::accept(Visitor& v) {
  v.visit(*this);
}

// Record

Record::Record() : record() {}

void Record::Add(string key, Expression *value) {
  this->record[key] = value;
}

void Record::accept(Visitor& v) {
  v.visit(*this);
}

// NullConstant

void NullConstant::accept(Visitor& v) {
  v.visit(*this);
}
