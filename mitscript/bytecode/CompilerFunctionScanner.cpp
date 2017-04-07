#include <iostream>
#include <string>
#include "CompilerFunctionScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;

namespace BC {
  void CompilerFunctionScanner::scan(AST::AST_node *node) {
    node->accept(*this);
  }

  void CompilerFunctionScanner::visit(AST::Program& prog) {}

  void CompilerFunctionScanner::visit(AST::Block& block) {
    for (Statement *stmt : block.statements) {
      scan(stmt);
    }
  }

  void CompilerFunctionScanner::visit(AST::Name& name) {
    if (
      index(current().free_vars_, name.name) ||
      index(current().local_vars_, name.name) ||
      index(current().names_, name.name)
    ) {
      return;
    }

    shared_ptr<FunctionLinkedList> node = functions->last;
    while (node) {
      if (index(node->function->local_vars_, name.name)) {
        insert(current().free_vars_, name.name);
        insert(node->local_reference_vars_, name.name);
        return;
      } else if (index(node->function->free_vars_, name.name)) {
        insert(current().free_vars_, name.name);
        insert(node->free_reference_vars_, name.name);
        return;
      } else if (index(node->function->names_, name.name)) {
        insert(current().names_, name.name);
        return;
      }
      node = node->last;
    }

    throw UninitializedVariableException(name.name);
  }

  void CompilerFunctionScanner::visit(AST::IndexExpression& ie) {
    scan(ie.base);
    scan(ie.index);
  }

  void CompilerFunctionScanner::visit(AST::FieldDereference& fd) {
    scan(fd.base);
    scan(fd.field);
  }

  void CompilerFunctionScanner::visit(AST::Assignment& assign) {
    if (Name *name = dynamic_cast<Name*>(assign.lhs)) {
      insert(current().local_vars_, name->name);
    } else {
      scan(assign.lhs);
    }
    scan(assign.expr);
  }

  void CompilerFunctionScanner::visit(AST::Call& call) {
    scan(call.target);
    for (auto const &arg : call.arguments)
      scan(arg);
  }

  void CompilerFunctionScanner::visit(AST::CallStatement& cs) {
    scan(cs.call);
  }

  void CompilerFunctionScanner::visit(AST::Global& global) {
    insert(current().names_, global.name);
  }

  void CompilerFunctionScanner::visit(AST::IfStatement& is) {
    scan(is.cond);
    scan(is.thenBlock);
    scan(is.elseBlock);
  }

  void CompilerFunctionScanner::visit(AST::WhileLoop& wl) {
    scan(wl.cond);
    scan(wl.body);
  }

  void CompilerFunctionScanner::visit(AST::Return& ret) {
    scan(ret.expr);
  }

  void CompilerFunctionScanner::visit(AST::Function& func) { }

  void CompilerFunctionScanner::visit(AST::Record& rec) {
    rec.record.iterate([this] (string key, Expression *value) { scan(value); });
  }

  void CompilerFunctionScanner::visit(AST::ValueConstant<bool>& boolconst) {}

  void CompilerFunctionScanner::visit(AST::StringConstant& strconst) {}

  void CompilerFunctionScanner::visit(AST::ValueConstant<int>& intconst) {}

  void CompilerFunctionScanner::visit(AST::NullConstant& nullconst) {}

  void CompilerFunctionScanner::visit(AST::BinaryOp<OR>& orop) {
    scan(orop.left);
    scan(orop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<AND>& andop) {
    scan(andop.left);
    scan(andop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<LT>& ltop) {
    scan(ltop.left);
    scan(ltop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<LTE>& lteop) {
    scan(lteop.left);
    scan(lteop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<GT>& gtop) {
    scan(gtop.left);
    scan(gtop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<GTE>& gteop) {
    scan(gteop.left);
    scan(gteop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<EQ>& eqop) {
    scan(eqop.left);
    scan(eqop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<PLUS>& plusop) {
    scan(plusop.left);
    scan(plusop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<MINUS>& minusop) {
    scan(minusop.left);
    scan(minusop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<MUL>& mulop) {
    scan(mulop.left);
    scan(mulop.right);
  }

  void CompilerFunctionScanner::visit(AST::BinaryOp<DIV>& divop) {
    scan(divop.left);
    scan(divop.right);
  }

  void CompilerFunctionScanner::visit(AST::UnaryOp<NOT>& notop) {
    scan(notop.expr);
  }

  void CompilerFunctionScanner::visit(AST::UnaryOp<NEG>& negop) {
    scan(negop.expr);
  }
}
