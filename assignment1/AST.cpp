#include <sstream>
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

bool Block::empty() {
  return this->statements.empty();
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

void Record::accept(Visitor& v) {
  v.visit(*this);
}

// StringConstant

StringConstant::StringConstant(string value) {
  if (value.find("\\") == string::npos) {
    this->value = value;
    return;
  }

  stringstream ss;
  for (size_t i = 0; i < value.size(); i++) {
    if (value[i] == '\\' && i + 1 < value.size()) {
      switch(value[i+1]) {
        case '\\':
          ss << "\\";
          break;
        case 'n':
          ss << "\n";
          break;
        case 't':
          ss << "\t";
          break;
        default:
          ss << "\\" << value[i+1];
          break;
      }
      i++;
    } else {
      ss << value[i];
    }
  }
  this->value = ss.str();
}

void StringConstant::accept(Visitor& v) {
  v.visit(*this);
}

// NullConstant

void NullConstant::accept(Visitor& v) {
  v.visit(*this);
}
