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
  Expression *expr;

  Assignment(LHS *lhs, Expression *expr) : lhs(lhs), expr(expr) {}
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
  Expression *cond;
  Block *thenBlock;
  Block *elseBlock;

  IfStatement(Expression *cond, Block *thenBlock, Block *elseBlock)
    : cond(cond), thenBlock(thenBlock), elseBlock(elseBlock)
  {}
};

class WhileLoop: public Statement {
public:
  Expression *cond;
  Block *body;

  WhileLoop(Expression *cond, Block *body) : cond(cond), body(body) {}
};

class Return: public Statement {
public:
  Expression *expr;

  Global(Expression *expr) : expr(expr) {}
};

// Expression

class Expression : public AST_node {
};

// Function

class Function: public Expression {
public:
  vector<Expression> arguments;
  Block *body;

  Function(vector<Expression> arguments, Block *body) : arguments(arguments), body(body) {}
};

// Record

class Record: public Expression {
  map<string, Expression> _map;

public:
  Record() : map() {}
  Add(string key, Expression value) {
    this._map[key] = value;
  }
};

// Ops

template <string op>
class BinaryOp : public Expression {
public:
  Expression *left;
  Expression *right;

  BinaryOp(Expression *left, Expression *right) : left(left), right(right) {}
};

template <string op>
class UnaryOp : public Expression {
public:
  Expression *expr;

  UnaryOp(Expression *expr) : expr(expr) {}
};


