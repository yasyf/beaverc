#include <iostream>
#include <string>
#include "TranspilerFunctionScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;

namespace BC {
  void TranspilerFunctionScanner::scan(AST::AST_node *node) {
    node->accept(*this);
  }

  void TranspilerFunctionScanner::visit(AST::Program& prog) {}

  void TranspilerFunctionScanner::visit(AST::Block& block) {
    for (Statement *stmt : block.statements) {
      scan(stmt);
    }
  }

  void TranspilerFunctionScanner::visit(AST::Name& name) {
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

  void TranspilerFunctionScanner::visit(AST::IndexExpression& ie) {
    scan(ie.base);
    scan(ie.index);
  }

  void TranspilerFunctionScanner::visit(AST::FieldDereference& fd) {
    scan(fd.base);
    scan(fd.field);
  }

  void TranspilerFunctionScanner::visit(AST::Assignment& assign) {
    if (Name *name = dynamic_cast<Name*>(assign.lhs)) {
      insert(current().local_vars_, name->name);
    } else {
      scan(assign.lhs);
    }
    scan(assign.expr);
  }

  void TranspilerFunctionScanner::visit(AST::Call& call) {
    scan(call.target);
    for (auto const &arg : call.arguments)
      scan(arg);
  }

  void TranspilerFunctionScanner::visit(AST::CallStatement& cs) {
    scan(cs.call);
  }

  void TranspilerFunctionScanner::visit(AST::Global& global) {
    insert(current().names_, global.name);
  }

  void TranspilerFunctionScanner::visit(AST::IfStatement& is) {
    scan(is.cond);
    scan(is.thenBlock);
    scan(is.elseBlock);
  }

  void TranspilerFunctionScanner::visit(AST::WhileLoop& wl) {
    scan(wl.cond);
    scan(wl.body);
  }

  void TranspilerFunctionScanner::visit(AST::Return& ret) {
    scan(ret.expr);
  }

  void TranspilerFunctionScanner::visit(AST::Function& func) { }

  void TranspilerFunctionScanner::visit(AST::Record& rec) {
    rec.record.iterate([this] (string key, Expression *value) { scan(value); });
  }

  void TranspilerFunctionScanner::visit(AST::ValueConstant<bool>& boolconst) {}

  void TranspilerFunctionScanner::visit(AST::StringConstant& strconst) {}

  void TranspilerFunctionScanner::visit(AST::ValueConstant<int>& intconst) {}

  void TranspilerFunctionScanner::visit(AST::NullConstant& nullconst) {}

  void TranspilerFunctionScanner::visit(AST::BinaryOp<OR>& orop) {
    scan(orop.left);
    scan(orop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<AND>& andop) {
    scan(andop.left);
    scan(andop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<LT>& ltop) {
    scan(ltop.left);
    scan(ltop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<LTE>& lteop) {
    scan(lteop.left);
    scan(lteop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<GT>& gtop) {
    scan(gtop.left);
    scan(gtop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<GTE>& gteop) {
    scan(gteop.left);
    scan(gteop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<EQ>& eqop) {
    scan(eqop.left);
    scan(eqop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<PLUS>& plusop) {
    scan(plusop.left);
    scan(plusop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<MINUS>& minusop) {
    scan(minusop.left);
    scan(minusop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<MUL>& mulop) {
    scan(mulop.left);
    scan(mulop.right);
  }

  void TranspilerFunctionScanner::visit(AST::BinaryOp<DIV>& divop) {
    scan(divop.left);
    scan(divop.right);
  }

  void TranspilerFunctionScanner::visit(AST::UnaryOp<NOT>& notop) {
    scan(notop.expr);
  }

  void TranspilerFunctionScanner::visit(AST::UnaryOp<NEG>& negop) {
    scan(negop.expr);
  }
}
