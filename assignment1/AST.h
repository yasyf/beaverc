#pragma once

#include <vector>
#include <map>
#include <string>
#include "op.h"
#include "Visitor.h"

class SystemException {
	std::string msg_;
public:
	SystemException(const std::string& msg) :msg_(msg) {}
};

#define Assert(cond, msg) if(!(cond)){ std::cerr<<msg<<endl; throw SystemException("Bad stuff"); }

class AST_node {
public:
	virtual void accept(Visitor& v) = 0;
};

class Expression : public AST_node {
};

class Unit : public Expression {
};

class Statement: public AST_node {
};

class Block: public AST_node {
public:
  std::vector<Statement *> statements;

  Block();
  void accept(Visitor& v) override;
  void Append(Statement *statement);
};

class Program : public AST_node {
public:
  Block *block;

  Program();
  void accept(Visitor& v) override;
};

// LHS

class LHS : public Unit {
};

class Name : public LHS {
public:
  std::string name;

  Name(std::string name);
  void accept(Visitor& v) override;
};

class IndexExpression: public LHS {
  LHS *base;
  Expression *index;

public:
  IndexExpression(LHS *base, Expression *index);
  void accept(Visitor& v) override;
};

class FieldDereference: public LHS {
  LHS *base;
  Name *field;

public:
  FieldDereference(LHS *base, Name *field);
  void accept(Visitor& v) override;
};

// Statement

class Assignment: public Statement {
public:
  LHS *lhs;
  Expression *expr;

  Assignment(LHS *lhs, Expression *expr);
  void accept(Visitor& v) override;
};

class Call: public Unit {
  LHS *target;
  std::vector<Expression *> arguments;

public:
  Call(LHS *target, std::vector<Expression *> arguments);
  void accept(Visitor& v) override;
};

class CallStatement: public Statement {
  Call *call;

public:
  CallStatement(Call *call);
  void accept(Visitor& v) override;
};

class Global: public Statement {
  std::string name;

public:
  Global(std::string name);
  void accept(Visitor& v) override;
};

class IfStatement: public Statement {
  Expression *cond;
  Block *thenBlock;
  Block *elseBlock;

public:
  IfStatement(Expression *cond, Block *thenBlock, Block *elseBlock);
  void accept(Visitor& v) override;
};

class WhileLoop: public Statement {
  Expression *cond;
  Block *body;

public:
  WhileLoop(Expression *cond, Block *body);
  void accept(Visitor& v) override;
};

class Return: public Statement {
  Expression *expr;

public:
  Return(Expression *expr);
  void accept(Visitor& v) override;
};

// Expression

class Constant : public Unit {
};

template <typename T>
class ValueConstant : public Constant {
public:
  T value;

  ValueConstant(T value) : value(value) {}
  void accept(Visitor& v) override {
    v.visit(*this);
  }
};

class NullConstant : public Constant {
public:
  NullConstant() {}
  void accept(Visitor& v) override;
};

// Function

class Function: public Expression {
  std::vector<Name *> arguments;
  Block *body;

public:
  Function(std::vector<Name *> arguments, Block *body);
  void accept(Visitor& v) override;
};

// Record

class Record: public Expression {
  std::map<std::string, Expression *> record;

public:
  Record();
  void accept(Visitor& v) override;
  void Add(std::string key, Expression *value);
};

// Ops

template <BinOpSym op>
class BinaryOp : public Expression {
public:
  Expression *left;
  Expression *right;

  BinaryOp(Expression *left, Expression *right) : left(left), right(right) {}
  void accept(Visitor& v) override {
    v.visit(*this);
  }
};

template <UnOpSym op>
class UnaryOp : public Expression {
public:
  Expression *expr;

  UnaryOp(Expression *expr) : expr(expr) {}
  void accept(Visitor& v) override {
    v.visit(*this);
  }
};
