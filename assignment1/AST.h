#pragma once

#include <vector>
#include <map>
#include <string>

class SystemException {
	std::string msg_;
public:
	SystemException(const std::string& msg) :msg_(msg) {}
};

#define Assert(cond, msg) if(!(cond)){ std::cerr<<msg<<endl; throw SystemException("Bad stuff"); }

#include "Visitor.h"

enum Op {OR, AND, NOT, LT, LTE, GT, GTE, EQ, PLUS, MINUS, MUL, DIV};

class AST_node {
public:
	// virtual void accept(Visitor& v) = 0;
};

class Expression : public AST_node {
};

class Unit : public Expression {
};

class Statement: public AST_node {
};

class Block: public AST_node {
  std::vector<Statement *> statements;

public:
  Block();
  void Append(Statement *statement);
};

class Program : public AST_node {
public:
  Block *block;

  Program();
};

// LHS

class LHS : public Unit {
};

class Name : public LHS {
  std::string name;

public:
  Name(std::string name);
};

class IndexExpression: public LHS {
  LHS *base;
  Expression *index;

public:
  IndexExpression(LHS *base, Expression *index);
};

class FieldDereference: public LHS {
  LHS *base;
  Name *field;

public:
  FieldDereference(LHS *base, Name *field);
};

// Statement

class Assignment: public Statement {
  LHS *lhs;
  Expression *expr;

public:
  Assignment(LHS *lhs, Expression *expr);
};

class Call: public Unit {
  LHS *target;
  std::vector<Expression *> arguments;

public:
  Call(LHS *target, std::vector<Expression *> arguments);
};

class CallStatement: public Statement {
  Call *call;

public:
  CallStatement(Call *call);
};

class Global: public Statement {
  std::string name;

public:
  Global(std::string name);
};

class IfStatement: public Statement {
  Expression *cond;
  Block *thenBlock;
  Block *elseBlock;

public:
  IfStatement(Expression *cond, Block *thenBlock, Block *elseBlock);
};

class WhileLoop: public Statement {
  Expression *cond;
  Block *body;

public:
  WhileLoop(Expression *cond, Block *body);
};

class Return: public Statement {
  Expression *expr;

public:
  Return(Expression *expr);
};

// Expression

template <class T>
class Constant : public Unit {
  T value;

public:
  Constant(T value) : value(value) {}
};

// Function

class Function: public Expression {
  std::vector<Name *> arguments;
  Block *body;

public:
  Function(std::vector<Name *> arguments, Block *body);
};

// Record

class Record: public Expression {
  std::map<std::string, Expression *> record;

public:
  Record();
  void Add(std::string key, Expression *value);
};

// Ops

template <Op op>
class BinaryOp : public Expression {
  Expression *left;
  Expression *right;

public:
  BinaryOp(Expression *left, Expression *right) : left(left), right(right) {}
};

template <Op op>
class UnaryOp : public Expression {
  Expression *expr;

public:
  UnaryOp(Expression *expr) : expr(expr) {}
};


