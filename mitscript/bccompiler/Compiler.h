#pragma once

#include "../Visitor.h"
#include "../AST.h"
#include "Types.h"

using namespace std;
using namespace AST;

namespace BC {
  class Compiler : public Visitor {
  protected:
    shared_ptr<FunctionLinkedList> root;
    shared_ptr<FunctionLinkedList> parents;
    bool storing = false;

    shared_ptr<FunctionLinkedList> parent() {
      return parents->last;
    }
    Function& current() {
      return *(parents->function);
    }
    Function& last() {
      return *(parent()->function);
    }
    bool isGlobal() {
      return parents->function == result;
    }
    void addNativeFunction(string name, size_t argc);
    void transpile(AST_node *node, bool storing = false, InstructionList* out = nullptr);
    void transpileTo(AST_node *node, InstructionList* out);
    void store(AST_node *node);
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
    void visitBinop(BinaryOp<op>& binop, Operation operation, bool reverse = false);
    template <UnOpSym op>
    void visitUnop(UnaryOp<op>& unop, Operation operation);

  public:
    shared_ptr<Function> result;

    Compiler();
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
