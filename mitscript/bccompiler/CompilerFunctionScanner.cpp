#include <iostream>
#include <string>
#include "CompilerFunctionScanner.h"
#include "Util.h"
#include "Exception.h"

using namespace std;
using namespace AST;

namespace BC {
  void CompilerFunctionScanner::scan(AST_node *node) {
    node->accept(*this);
  }

  void CompilerFunctionScanner::visit(Program& prog) {}

  void CompilerFunctionScanner::visit(Block& block) {
    for (Statement *stmt : block.statements) {
      scan(stmt);
    }
  }

  void CompilerFunctionScanner::visit(Name& name) {
    if (
      index(current().free_vars_, name.name) ||
      index(current().local_vars_, name.name) ||
      index(current().names_, name.name)
    ) {
      return;
    }

    shared_ptr<FunctionLinkedList> node = functions->last;
    shared_ptr<FunctionLinkedList> last_node;
    vector<shared_ptr<FunctionLinkedList>> ancestors;

    while (node) {
      if (index(node->function->local_vars_, name.name)) {
        insert(current().free_vars_, name.name);
        insert(node->local_reference_vars_, name.name);
        for (auto an : ancestors)
          insert(an->free_reference_vars_, name.name);
        return;
      } else if (index(node->function->free_vars_, name.name)) {
        insert(current().free_vars_, name.name);
        insert(node->free_reference_vars_, name.name);
        for (auto an : ancestors)
          insert(an->free_reference_vars_, name.name);
        return;
      } else if (index(node->function->names_, name.name)) {
        insert(current().names_, name.name);
        return;
      }

      ancestors.push_back(node);
      last_node = node;
      node = node->last;
    }

    // Special case for recursive define at top level
    if (last_node->storing == name.name)
      insert(current().names_, name.name);
    else
      throw UninitializedVariableException(name.name);
  }

  void CompilerFunctionScanner::visit(IndexExpression& ie) {
    scan(ie.base);
    scan(ie.index);
  }

  void CompilerFunctionScanner::visit(FieldDereference& fd) {
    scan(fd.base);
    scan(fd.field);
  }

  void CompilerFunctionScanner::visit(Assignment& assign) {
    if (Name *name = dynamic_cast<Name*>(assign.lhs)) {
      insert(current().local_vars_, name->name);
    } else {
      scan(assign.lhs);
    }
    scan(assign.expr);
  }

  void CompilerFunctionScanner::visit(Call& call) {
    scan(call.target);
    for (auto const &arg : call.arguments)
      scan(arg);
  }

  void CompilerFunctionScanner::visit(CallStatement& cs) {
    scan(cs.call);
  }

  void CompilerFunctionScanner::visit(Global& global) {
    insert(current().names_, global.name);
  }

  void CompilerFunctionScanner::visit(IfStatement& is) {
    scan(is.cond);
    scan(is.thenBlock);
    scan(is.elseBlock);
  }

  void CompilerFunctionScanner::visit(WhileLoop& wl) {
    scan(wl.cond);
    scan(wl.body);
  }

  void CompilerFunctionScanner::visit(Return& ret) {
    scan(ret.expr);
  }

  void CompilerFunctionScanner::visit(AST::Function& func) { }

  void CompilerFunctionScanner::visit(Record& rec) {
    rec.record.iterate([this] (string key, Expression *value) { scan(value); });
  }

  void CompilerFunctionScanner::visit(ValueConstant<bool>& boolconst) {}

  void CompilerFunctionScanner::visit(StringConstant& strconst) {}

  void CompilerFunctionScanner::visit(ValueConstant<int>& intconst) {}

  void CompilerFunctionScanner::visit(NullConstant& nullconst) {}

  void CompilerFunctionScanner::visit(BinaryOp<OR>& orop) {
    scan(orop.left);
    scan(orop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<AND>& andop) {
    scan(andop.left);
    scan(andop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<LT>& ltop) {
    scan(ltop.left);
    scan(ltop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<LTE>& lteop) {
    scan(lteop.left);
    scan(lteop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<GT>& gtop) {
    scan(gtop.left);
    scan(gtop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<GTE>& gteop) {
    scan(gteop.left);
    scan(gteop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<EQ>& eqop) {
    scan(eqop.left);
    scan(eqop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<PLUS>& plusop) {
    scan(plusop.left);
    scan(plusop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<MINUS>& minusop) {
    scan(minusop.left);
    scan(minusop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<MUL>& mulop) {
    scan(mulop.left);
    scan(mulop.right);
  }

  void CompilerFunctionScanner::visit(BinaryOp<DIV>& divop) {
    scan(divop.left);
    scan(divop.right);
  }

  void CompilerFunctionScanner::visit(UnaryOp<NOT>& notop) {
    scan(notop.expr);
  }

  void CompilerFunctionScanner::visit(UnaryOp<NEG>& negop) {
    scan(negop.expr);
  }
}
