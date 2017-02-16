#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>



class SystemException {
	string msg_;
public:
	SystemException(const string& msg) :msg_(msg) {}
};

#define Assert(cond, msg) if(!(cond)){ std::cerr<<msg<<endl; throw SystemException("Bad stuff"); }

using namespace std;


#include "Visitor.h"



class AST_node {

public:
	virtual void accept(Visitor& v) = 0;
};

class Program : public AST_node {
public:
  Block *block;

  Program() : block(new Block()) { }
};

class Block: public AST_node {
  vector<Statement *> _statements;

public:
  Block() : _statements() {}

  void Append(Statement *statement) {
    this->_statements.push_back(statement);
  }
};

// LHS

class LHS: public AST_node {
public:
  string name;

  LHS(string name) : name(name) {}
};

class IndexExpression: public LHS {
public:
  LHS *base;
  Expression *index;

  IndexExpression(LHS *base, Expression *index) : base(base), index(index) {}
};

class FieldDereference: public LHS {
public:
  LHS *base;
  string field;

  FieldDereference(LHS *base, string field) : base(base), field(field) {}
};

// Statement

class Statement: public AST_node {

};

class Assignment: public Statement {
public:
  LHS *lhs;
  Expression *expression;

  Assignment(LHS *lhs, Expression *expression) : lhs(lhs), expression(expression) {}
};

class CallStatement: public Statement {
public:
  LHS *target;
  vector<Expression> arguments;

  CallStatement(LHS *target, vector<Expression> arguments) : target(target), arguments(arguments) {}
};

class Global: public Statement {
public:
  string name;

  Global(string name) : name(name) {}
};

class IfStatement: public Statement {
public:
  Expression *condition;
  Block *thenBlock;
  Block *elseBlock;

  IfStatement(Expression *condition, Block *thenBlock, Block *elseBlock)
    : condition(condition), thenBlock(thenBlock), elseBlock(elseBlock)
  {}
};

class WhileLoop: public Statement {
public:
  Expression *condition;
  Block *body;

  WhileLoop(Expression *condition, Block *body) : condition(condition), body(body) {}
};

class Return: public Statement {
public:
  Expression *expression;

  Global(Expression *expression) : expression(expression) {}
};

// Expression

class Expression : public AST_node {
public:

};

class Function: public Expression {

};

class BooleanExpr: public Expression {

};

class Record: public Expression {

};

