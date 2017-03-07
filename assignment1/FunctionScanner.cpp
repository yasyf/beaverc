#include "FunctionScanner.h"

using namespace std;

FunctionScanner::FunctionScanner(StackFrame *stack) : stack(stack) {}

void FunctionScanner::visit(Program& prog) {}

void FunctionScanner::visit(Block& block) {
  for (Statement *stmt : block.statements) {
    stmt->accept(*this);
  }
}

void FunctionScanner::visit(Name& name) {}

void FunctionScanner::visit(IndexExpression& ie) {}

void FunctionScanner::visit(FieldDereference& fd) {}

void FunctionScanner::visit(Assignment& assign) {
  if (Name *name = dynamic_cast<Name*>(assign.lhs)) {
    stack->Initialize(name->name);
  }
}

void FunctionScanner::visit(Call& call) {}

void FunctionScanner::visit(CallStatement& cs) {}

void FunctionScanner::visit(Global& global) {
  stack->globals.insert(global.name);
}

void FunctionScanner::visit(IfStatement& is) {
  is.thenBlock->accept(*this);
  is.elseBlock->accept(*this);
}

void FunctionScanner::visit(WhileLoop& wl) {
  wl.body->accept(*this);
}

void FunctionScanner::visit(Return& ret) {}

void FunctionScanner::visit(Function& func) {}

void FunctionScanner::visit(Record& rec) {}

void FunctionScanner::visit(ValueConstant<bool>& boolconst) {}

void FunctionScanner::visit(ValueConstant<std::string>& strconst) {}

void FunctionScanner::visit(ValueConstant<int>& intconst) {}

void FunctionScanner::visit(NullConstant& nullconst) {}

void FunctionScanner::visit(BinaryOp<OR>& orop) {}

void FunctionScanner::visit(BinaryOp<AND>& andop) {}

void FunctionScanner::visit(BinaryOp<LT>& ltop) {}

void FunctionScanner::visit(BinaryOp<LTE>& lteop) {}

void FunctionScanner::visit(BinaryOp<GT>& gtop) {}

void FunctionScanner::visit(BinaryOp<GTE>& gteop) {}

void FunctionScanner::visit(BinaryOp<EQ>& eqop) {}

void FunctionScanner::visit(BinaryOp<PLUS>& plusop) {}

void FunctionScanner::visit(BinaryOp<MINUS>& minusop) {}

void FunctionScanner::visit(BinaryOp<MUL>& mulop) {}

void FunctionScanner::visit(BinaryOp<DIV>& divop) {}

void FunctionScanner::visit(UnaryOp<NOT>& notop) {}

void FunctionScanner::visit(UnaryOp<NEG>& negop) {}
