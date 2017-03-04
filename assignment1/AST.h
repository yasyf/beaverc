#pragma once

#include <vector>
#include <map>
#include <string>
#include "Op.h"
#include "Visitor.h"

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
  bool empty();
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
public:
  LHS *base;
  Expression *index;

  IndexExpression(LHS *base, Expression *index);
  void accept(Visitor& v) override;
};

class FieldDereference: public LHS {
public:
  LHS *base;
  Name *field;

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
public:
  LHS *target;
  std::vector<Expression *> arguments;

  Call(LHS *target, std::vector<Expression *> arguments);
  void accept(Visitor& v) override;
};

class CallStatement: public Statement {
public:
  Call *call;

  CallStatement(Call *call);
  void accept(Visitor& v) override;
};

class Global: public Statement {
public:
  std::string name;

  Global(std::string name);
  void accept(Visitor& v) override;
};

class IfStatement: public Statement {
public:
  Expression *cond;
  Block *thenBlock;
  Block *elseBlock;

  IfStatement(Expression *cond, Block *thenBlock, Block *elseBlock);
  void accept(Visitor& v) override;
};

class WhileLoop: public Statement {
public:
  Expression *cond;
  Block *body;

  WhileLoop(Expression *cond, Block *body);
  void accept(Visitor& v) override;
};

class Return: public Statement {
public:
  Expression *expr;

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
public:
  std::vector<Name *> arguments;
  Block *body;

  Function(std::vector<Name *> arguments, Block *body);
  void accept(Visitor& v) override;
};

// Record

class Record: public Expression {
public:
  std::map<std::string, Expression *> record;

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
