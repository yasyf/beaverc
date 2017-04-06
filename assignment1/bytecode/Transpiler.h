#pragma once

#include "../Visitor.h"
#include "../AST.h"
#include "Types.h"

using namespace std;

namespace BC {
  class Transpiler : public AST::Visitor {
  protected:
    shared_ptr<FunctionLinkedList> parents;
    InstructionList* out = nullptr;
    bool storing = false;

    Function& current() {
      return *(parents->function);
    }
    bool isGlobal() {
      return parents->function == result;
    }
    void transpile(AST::AST_node *node, bool storing = false, InstructionList* out = nullptr);
    void transpileTo(AST::AST_node *node, InstructionList* out);
    void store(AST::AST_node *node);
    size_t count();
    void output(Instruction inst);
    void output(const Operation operation);
    void output(const Operation operation, int32_t operand0);
    void drain(InstructionList il);
    void loadConst(shared_ptr<Constant> constant);
    void loadConst(int i);
    void loadConst(string s);
    void loadBool(bool b);
    void loadNone();
    void outputReturn();

    template <BinOpSym op>
    void visitBinop(AST::BinaryOp<op>& binop, Operation operation, bool reverse = false);
    template <UnOpSym op>
    void visitUnop(AST::UnaryOp<op>& unop, Operation operation);

  public:
    shared_ptr<Function> result;

    Transpiler();
    void visit(AST::Program& prog) override;
    void visit(AST::Block& block) override;
    void visit(AST::Name& name) override;
    void visit(AST::IndexExpression& ie) override;
    void visit(AST::FieldDereference& fd) override;
    void visit(AST::Assignment& assign) override;
    void visit(AST::Call& call) override;
    void visit(AST::CallStatement& cs) override;
    void visit(AST::Global& global) override;
    void visit(AST::IfStatement& is) override;
    void visit(AST::WhileLoop& wl) override;
    void visit(AST::Return& ret) override;
    void visit(AST::Function& func) override;
    void visit(AST::Record& rec) override;
    void visit(AST::ValueConstant<bool>& boolconst) override;
    void visit(AST::StringConstant& strconst) override;
    void visit(AST::ValueConstant<int>& intconst) override;
    void visit(AST::NullConstant& nullconst) override;
    void visit(AST::BinaryOp<OR>& orop) override;
    void visit(AST::BinaryOp<AND>& andop) override;
    void visit(AST::BinaryOp<LT>& ltop) override;
    void visit(AST::BinaryOp<LTE>& lteop) override;
    void visit(AST::BinaryOp<GT>& gtop) override;
    void visit(AST::BinaryOp<GTE>& gteop) override;
    void visit(AST::BinaryOp<EQ>& eqop) override;
    void visit(AST::BinaryOp<PLUS>& plusop) override;
    void visit(AST::BinaryOp<MINUS>& minusop) override;
    void visit(AST::BinaryOp<MUL>& mulop) override;
    void visit(AST::BinaryOp<DIV>& divop) override;
    void visit(AST::UnaryOp<NOT>& notop) override;
    void visit(AST::UnaryOp<NEG>& negop) override;
  };
}
