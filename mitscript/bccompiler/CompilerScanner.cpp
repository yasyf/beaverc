#include <iostream>
#include <string>
#include "CompilerScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;
using namespace AST;

namespace BC {
  void CompilerScanner::scan(AST_node *node, bool assigning) {
    bool old_assigning = this->assigning;
    this->assigning = assigning;
    node->accept(*this);
    this->assigning = old_assigning;
  }

  void CompilerScanner::visit(Program& prog) {}

  void CompilerScanner::visit(Block& block) {
    for (Statement *stmt : block.statements) {
      scan(stmt);
    }
  }

  void CompilerScanner::visit(Name& name) { }

  void CompilerScanner::visit(IndexExpression& ie) {
    scan(ie.base);
    scan(ie.index);
  }

  void CompilerScanner::visit(FieldDereference& fd) {
    scan(fd.base);
    scan(fd.field);
  }

  void CompilerScanner::visit(Assignment& assign) {
    scan(assign.lhs, true);
    scan(assign.expr);
  }

  void CompilerScanner::visit(Call& call) {
    scan(call.target);
    for (auto const &arg : call.arguments)
      scan(arg);
  }

  void CompilerScanner::visit(CallStatement& cs) {
    scan(cs.call);
  }

  void CompilerScanner::visit(Global& global) { }

  void CompilerScanner::visit(IfStatement& is) {
    scan(is.cond);
    scan(is.thenBlock);
    scan(is.elseBlock);
  }

  void CompilerScanner::visit(WhileLoop& wl) {
    scan(wl.cond);
    scan(wl.body);
  }

  void CompilerScanner::visit(Return& ret) {
    scan(ret.expr);
  }

  void CompilerScanner::visit(AST::Function& func) {
    for (Name *name : func.arguments) {
      scan(name);
    }
    scan(func.body);
  }

  void CompilerScanner::visit(Record& rec) {
    rec.record.iterate([this] (string key, Expression *value) { scan(value); });
  }

  void CompilerScanner::visit(ValueConstant<bool>& boolconst) {}

  void CompilerScanner::visit(StringConstant& strconst) {}

  void CompilerScanner::visit(ValueConstant<int>& intconst) {}

  void CompilerScanner::visit(NullConstant& nullconst) {}

  void CompilerScanner::visit(BinaryOp<OR>& orop) {
    scan(orop.left);
    scan(orop.right);
  }

  void CompilerScanner::visit(BinaryOp<AND>& andop) {
    scan(andop.left);
    scan(andop.right);
  }

  void CompilerScanner::visit(BinaryOp<LT>& ltop) {
    scan(ltop.left);
    scan(ltop.right);
  }

  void CompilerScanner::visit(BinaryOp<LTE>& lteop) {
    scan(lteop.left);
    scan(lteop.right);
  }

  void CompilerScanner::visit(BinaryOp<GT>& gtop) {
    scan(gtop.left);
    scan(gtop.right);
  }

  void CompilerScanner::visit(BinaryOp<GTE>& gteop) {
    scan(gteop.left);
    scan(gteop.right);
  }

  void CompilerScanner::visit(BinaryOp<EQ>& eqop) {
    scan(eqop.left);
    scan(eqop.right);
  }

  void CompilerScanner::visit(BinaryOp<PLUS>& plusop) {
    scan(plusop.left);
    scan(plusop.right);
  }

  void CompilerScanner::visit(BinaryOp<MINUS>& minusop) {
    scan(minusop.left);
    scan(minusop.right);
  }

  void CompilerScanner::visit(BinaryOp<MUL>& mulop) {
    scan(mulop.left);
    scan(mulop.right);
  }

  void CompilerScanner::visit(BinaryOp<DIV>& divop) {
    scan(divop.left);
    scan(divop.right);
  }

  void CompilerScanner::visit(UnaryOp<NOT>& notop) {
    scan(notop.expr);
  }

  void CompilerScanner::visit(UnaryOp<NEG>& negop) {
    scan(negop.expr);
  }
}
