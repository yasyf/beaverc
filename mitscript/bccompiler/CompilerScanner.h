#pragma once

#include "../Visitor.h"
#include "../AST.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class CompilerScanner : public Visitor {
  protected:
    bool assigning = false;
  public:
    void scan(AST_node *node, bool assigning = false);
    void visit(Program& prog) override;
    void visit(Block& block) override;
    void visit(Name& name) override;
    void visit(IndexExpression& ie) override;
    void visit(FieldDereference& fd) override;
    void visit(Assignment& assign) override;
    void visit(Call& call) override;
    void visit(CallStatement& cs) override;
    void visit(Global& global) override;
    void visit(IfStatement& is) override;
    void visit(WhileLoop& wl) override;
    void visit(Return& ret) override;
    void visit(AST::Function& func) override;
    void visit(Record& rec) override;
    void visit(ValueConstant<bool>& boolconst) override;
    void visit(StringConstant& strconst) override;
    void visit(ValueConstant<int>& intconst) override;
    void visit(NullConstant& nullconst) override;
    void visit(BinaryOp<OR>& orop) override;
    void visit(BinaryOp<AND>& andop) override;
    void visit(BinaryOp<LT>& ltop) override;
    void visit(BinaryOp<LTE>& lteop) override;
    void visit(BinaryOp<GT>& gtop) override;
    void visit(BinaryOp<GTE>& gteop) override;
    void visit(BinaryOp<EQ>& eqop) override;
    void visit(BinaryOp<PLUS>& plusop) override;
    void visit(BinaryOp<MINUS>& minusop) override;
    void visit(BinaryOp<MUL>& mulop) override;
    void visit(BinaryOp<DIV>& divop) override;
    void visit(UnaryOp<NOT>& notop) override;
    void visit(UnaryOp<NEG>& negop) override;
  };
}
