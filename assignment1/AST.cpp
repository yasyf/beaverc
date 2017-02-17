#include "AST.h"

using namespace std;

// Program

Program::Program() : block(new Block()) {}

// Block

Block::Block() : statements() {}

void Block::Append(Statement *statement) {
  this->statements.push_back(statement);
}

// Name

Name::Name(string name) : name(name) {}

// IndexExpression

IndexExpression::IndexExpression(LHS *base, Expression *index) : base(base), index(index) {}

// FieldDereference

FieldDereference::FieldDereference(LHS *base, Name *field) : base(base), field(field) {}

// Assignment

Assignment::Assignment(LHS *lhs, Expression *expr) : lhs(lhs), expr(expr) {}

// Call

Call::Call(LHS *target, vector<Expression *> arguments) : target(target), arguments(arguments) {}

// CallStatement

CallStatement::CallStatement(Call *call) : call(call) {}

// Global

Global::Global(string name) : name(name) {}

// IfStatement

IfStatement::IfStatement(Expression *cond, Block *thenBlock, Block *elseBlock)
  : cond(cond), thenBlock(thenBlock), elseBlock(elseBlock)
{}

// WhileLoop

WhileLoop::WhileLoop(Expression *cond, Block *body) : cond(cond), body(body) {}

// Return

Return::Return(Expression *expr) : expr(expr) {}

// Function

Function::Function(vector<Name *> arguments, Block *body) : arguments(arguments), body(body) {}

// Record

Record::Record() : record() {}

void Record::Add(string key, Expression *value) {
  this->record[key] = value;
}
