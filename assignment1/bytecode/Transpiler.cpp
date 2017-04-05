#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <experimental/optional>
#include "Util.h"
#include "Transpiler.h"
#include "Exception.h"
#include "TranspilerFunctionScanner.h"

#define DEBUG 1

using namespace std;
using namespace std::experimental;

namespace BC {
  Transpiler::Transpiler() {
    result = shared_ptr<Function>(new Function());
    result->parameter_count_ = 0;

    parents = shared_ptr<FunctionLinkedList>(new FunctionLinkedList(result));
  }

  void Transpiler::transpile(AST_node *node, bool storing) {
    bool old_storing = this->storing;
    this->storing = storing;
    node->accept(*this);
    this->storing = old_storing;
  }

  void Transpiler::store(AST_node *node) {
    transpile(node, true);
  }

  void Transpiler::output(const Operation operation) {
    #if DEBUG
      cout << static_cast<int>(operation) << endl;
    #endif
    current().instructions.push_back(Instruction(operation, nullopt));
  }

  void Transpiler::output(const Operation operation, int32_t operand0) {
    #if DEBUG
      cout << static_cast<int>(operation) << " " <<  operand0 << endl;
    #endif
    current().instructions.push_back(Instruction(operation, operand0));
  }

  void Transpiler::visit(Program& prog) {
    transpile(prog.block);
  }

  void Transpiler::visit(AST::Block& block) {
    for (Statement *stmt : block.statements) {
      transpile(stmt);
    }
  }

  void Transpiler::visit(AST::Name& name) {
    if (auto i = index(current().names_, name.name)) {
      if (storing)
        output(Operation::StoreGlobal, *i);
      else
        output(Operation::LoadGlobal, *i);
    } else if (auto i = index(current().free_vars_, name.name)) {
      output(Operation::PushReference, current().local_reference_vars_.size() + *i);
      if (storing) {
        output(Operation::Swap);
        output(Operation::StoreReference);
      } else {
        output(Operation::LoadReference);
      }
    } else if (auto i = index(current().local_reference_vars_, name.name)) {
      output(Operation::PushReference, *i);
      if (storing) {
        output(Operation::Swap);
        output(Operation::StoreReference);
      } else {
        output(Operation::LoadReference);
      }
    } else if (auto i = index(current().local_vars_, name.name)) {
      if (storing)
        output(Operation::StoreLocal, *i);
      else
        output(Operation::LoadLocal, *i);
    } else {
      if (storing && isGlobal()) {
        size_t i = insert(current().names_, name.name);
        output(Operation::StoreGlobal, i);
      } else {
        throw UninitializedVariableException(name.name);
      }
    }
  }

  void Transpiler::visit(AST::IndexExpression& ie) {
  }

  void Transpiler::visit(AST::FieldDereference& fd) {
  }

  void Transpiler::visit(AST::Assignment& assign) {
    // Leaves value at top of stack
    transpile(assign.expr);
    // Handlers will know to store value instead of loading
    store(assign.lhs);
  }

  void Transpiler::visit(AST::Call& call) {
  }

  void Transpiler::visit(AST::CallStatement& cs) {
  }

  void Transpiler::visit(AST::Global& global) {
  }

  void Transpiler::visit(AST::IfStatement& is) {
  }

  void Transpiler::visit(AST::WhileLoop& wl) {
  }

  void Transpiler::visit(AST::Return& ret) {
  }

  void Transpiler::visit(AST::Function& func) {
    shared_ptr<Function> function(new Function());
    function->parameter_count_ = func.arguments.size();
    for (AST::Name* name : func.arguments)
      insert(function->local_vars_, name->name);

    // Set current context to new function
    current().functions_.push_back(function);
    parents->reset_reference_vars();
    parents = parents->extend(function);

    // Fill new function metadata
    TranspilerFunctionScanner scanner(parents);
    func.body->accept(scanner);

    // Transpile new function
    transpile(func.body);

    // Restore parent function's context
    parents = parents->last;

    // Push Refs
    for (string& var : parents->local_reference_vars_) {
      size_t i = insert(current().local_reference_vars_, var);
      output(Operation::PushReference, i);
    }
    size_t num_local_refs = current().local_reference_vars_.size();
    for (string& var : parents->free_reference_vars_) {
      size_t i = index(current().free_vars_, var).value();
      output(Operation::PushReference, num_local_refs + i);
    }

    // Push num_refs
    size_t num_refs = parents->local_reference_vars_.size() + parents->free_reference_vars_.size();
    if (num_refs > 0) {
      shared_ptr<Constant> num_refs_const(new Integer(num_refs));
      size_t i = insert(current().constants_, num_refs_const);
      output(Operation::LoadConst, i);
    }

    // Push function
    output(Operation::LoadFunc, current().functions_.size() - 1);

    // Alloc closure if needed
    if (num_refs > 0) {
      output(Operation::AllocClosure);
    }
  }

  void Transpiler::visit(AST::Record& rec) {
  }

  void Transpiler::visit(AST::ValueConstant<bool>& boolconst) {
    shared_ptr<Constant> constant(new Boolean(boolconst.value));
    size_t i = insert(current().constants_, constant);
    output(Operation::LoadConst, i);
  }

  void Transpiler::visit(AST::StringConstant& strconst) {
    shared_ptr<Constant> constant(new String(strconst.value));
    size_t i = insert(current().constants_, constant);
    output(Operation::LoadConst, i);
  }

  void Transpiler::visit(AST::ValueConstant<int>& intconst) {
    shared_ptr<Constant> constant(new Integer(intconst.value));
    size_t i = insert(current().constants_, constant);
    output(Operation::LoadConst, i);
  }

  void Transpiler::visit(AST::NullConstant& nullconst) {
    shared_ptr<Constant> constant(new None());
    size_t i = insert(current().constants_, constant);
    output(Operation::LoadConst, i);
  }

  template <BinOpSym op>
  void Transpiler::visitBinop(AST::BinaryOp<op>& binop, string opstring) {
  }

  template <UnOpSym op>
  void Transpiler::visitUnop(AST::UnaryOp<op>& unop, string opstring) {
  }

  void Transpiler::visit(AST::BinaryOp<OR>& orop) {
    this->visitBinop(orop, "|");
  }

  void Transpiler::visit(AST::BinaryOp<AND>& andop) {
    this->visitBinop(andop, "&");
  }

  void Transpiler::visit(AST::BinaryOp<LT>& ltop) {
    this->visitBinop(ltop, "<");
  }

  void Transpiler::visit(AST::BinaryOp<LTE>& lteop) {
    this->visitBinop(lteop, "<=");
  }

  void Transpiler::visit(AST::BinaryOp<GT>& gtop) {
    this->visitBinop(gtop, ">");
  }

  void Transpiler::visit(AST::BinaryOp<GTE>& gteop) {
    this->visitBinop(gteop, ">=");
  }

  void Transpiler::visit(AST::BinaryOp<EQ>& eqop) {
    this->visitBinop(eqop, "==");
  }

  void Transpiler::visit(AST::BinaryOp<PLUS>& plusop) {
    this->visitBinop(plusop, "+");
  }

  void Transpiler::visit(AST::BinaryOp<MINUS>& minusop) {
    this->visitBinop(minusop, "-");
  }

  void Transpiler::visit(AST::BinaryOp<MUL>& mulop) {
    this->visitBinop(mulop, "*");
  }

  void Transpiler::visit(AST::BinaryOp<DIV>& divop) {
    this->visitBinop(divop, "/");
  }

  void Transpiler::visit(AST::UnaryOp<NOT>& notop) {
    this->visitUnop(notop, "!");
  }

  void Transpiler::visit(AST::UnaryOp<NEG>& negop) {
    this->visitUnop(negop, "-");
  }
}
