#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <experimental/optional>
#include "Util.h"
#include "Compiler.h"
#include "Exception.h"
#include "CompilerFunctionScanner.h"

#define DEBUG 0

using namespace std;
using namespace std::experimental;
using namespace AST;

namespace BC {
  const vector<pair<string, int>> Builtins = {{"print", 1}, {"input", 0}, {"intcast", 1}};

  Compiler::Compiler() {
    result = shared_ptr<Function>(new Function());
    result->parameter_count_ = 0;

    parents = shared_ptr<FunctionLinkedList>(new FunctionLinkedList(result));

    for (auto const& b : Builtins) {
      addNativeFunction(b.first, b.second);
    }
  }

  void Compiler::addNativeFunction(string name, size_t argc) {
    shared_ptr<Function> function(new Function());
    function->parameter_count_ = argc;
    current().functions_.push_back(function);
    this->parents = parents->extend(function);
    loadNone();
    outputReturn();
    this->parents = parents->last;

    output(Operation::LoadFunc, current().functions_.size() - 1);
    output(Operation::StoreGlobal, insert(current().names_, name));
  }

  void Compiler::transpile(AST_node *node, bool storing, InstructionList* out) {
    bool old_storing = this->storing;
    this->storing = storing;
    if (out)
      this->out = out;

    node->accept(*this);

    if (out)
      this->out = nullptr;
    this->storing = old_storing;
  }

  void Compiler::transpileTo(AST_node *node, InstructionList* out) {
    transpile(node, false, out);
  }

  void Compiler::store(AST_node *node) {
    transpile(node, true);
  }

  size_t Compiler::count() {
    return (out ? *out : current().instructions).size();
  }

  void Compiler::output(Instruction inst) {
    (out ? *out : current().instructions).push_back(inst);
  }

  void Compiler::output(const Operation operation) {
    #if DEBUG
      cout << static_cast<int>(operation) << endl;
    #endif
    output(Instruction(operation, nullopt));
  }

  void Compiler::output(const Operation operation, int32_t operand0) {
    #if DEBUG
      cout << static_cast<int>(operation) << " " <<  operand0 << endl;
    #endif
    output(Instruction(operation, operand0));
  }

  void Compiler::drain(InstructionList il) {
    for (auto inst : il)
      output(inst);
  }

  void Compiler::loadConst(shared_ptr<Constant> constant) {
    size_t i = insert_by_val(current().constants_, constant);
    output(Operation::LoadConst, i);
  }

  void Compiler::loadConst(int32_t i) {
    loadConst(shared_ptr<Constant>(new Integer(i)));
  }

  void Compiler::loadConst(string s) {
    loadConst(shared_ptr<Constant>(new String(s)));
  }

  void Compiler::loadBool(bool b) {
    loadConst(shared_ptr<Constant>(new Boolean(b)));
  }

  void Compiler::loadNone() {
    loadConst(shared_ptr<Constant>(new None()));
  }

  void Compiler::outputReturn() {
    output(Operation::Return);
    parents->returned = true;
  }

  void Compiler::visit(Program& prog) {
    transpile(prog.block);
    if (!parents->returned) {
      loadConst(0);
      outputReturn();
    }
  }

  void Compiler::visit(Block& block) {
    for (Statement *stmt : block.statements) {
      transpile(stmt);
    }
  }

  void Compiler::visit(Name& name) {
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

  void Compiler::visit(IndexExpression& ie) {
    if (storing) {
      transpile(ie.base);  // S :: value :: record
      output(Operation::Swap); // S :: record :: value
      transpile(ie.index); // S :: record :: value :: index
      output(Operation::Swap); // S :: record :: index :: value
      output(Operation::IndexStore);
    } else {
      transpile(ie.base);
      transpile(ie.index);
      output(Operation::IndexLoad);
    }
  }

  void Compiler::visit(FieldDereference& fd) {
    size_t i = insert(current().names_, fd.field->name);
    if (storing) {
      transpile(fd.base);  // S :: value :: record
      output(Operation::Swap); // S :: record :: value
      output(Operation::FieldStore, i);
    } else {
      transpile(fd.base);
      output(Operation::FieldLoad, i);
    }
  }

  void Compiler::visit(Assignment& assign) {
    // Inform scanner of current assignment
    if (Name *name = dynamic_cast<Name *>(assign.lhs))
      parents->storing = name->name;

    // Leaves value at top of stack
    transpile(assign.expr);
    // Handlers will know to store value instead of loading
    store(assign.lhs);
  }

  void Compiler::visit(Call& call) {
    for (auto it = call.arguments.rbegin(); it != call.arguments.rend(); ++it)
      transpile(*it);
    loadConst(call.arguments.size());
    transpile(call.target);
    output(Operation::Call);
  }

  void Compiler::visit(CallStatement& cs) {
    transpile(cs.call);
    output(Operation::Pop);
  }

  void Compiler::visit(Global& global) {
    // noop
  }

  void Compiler::visit(IfStatement& is) {
    // Set up blocks
    InstructionList thenInst, elseInst;
    transpileTo(is.thenBlock, &thenInst);
    transpileTo(is.elseBlock, &elseInst);

    // Execute if
    transpile(is.cond);
    output(Operation::If, 2); // skip past else-goto
    output(Operation::Goto, thenInst.size() + 2); // else-goto: skip past then-block and if-goto
    drain(thenInst); // then-block
    output(Operation::Goto, elseInst.size() + 1); // if-goto: skip past else-block
    drain(elseInst); // else-block
  }

  void Compiler::visit(WhileLoop& wl) {
    InstructionList bodyInst, condInst;
    transpileTo(wl.body, &bodyInst);
    transpileTo(wl.cond, &condInst);

    size_t loop_start = count();

    drain(condInst); // cond-block
    output(Operation::If, 2); // skip past end-goto
    output(Operation::Goto, bodyInst.size() + 2); // end-goto: skip past body-block and loop-goto
    drain(bodyInst); // body-block
    output(Operation::Goto, loop_start - count()); // loop-goto: back up to cond-block
  }

  void Compiler::visit(Return& ret) {
    transpile(ret.expr);
    outputReturn();
  }

  void Compiler::visit(AST::Function& func) {
    shared_ptr<Function> function(new Function());
    function->parameter_count_ = func.arguments.size();
    for (Name* name : func.arguments)
      insert(function->local_vars_, name->name);

    // Set current context to new function
    current().functions_.push_back(function);
    parents->reset_reference_vars();
    parents = parents->extend(function);

    // Fill new function metadata
    CompilerFunctionScanner scanner(parents);
    func.body->accept(scanner);

    // Transpile new function
    transpile(func.body);

    // Default return
    if (!parents->returned) {
      loadNone();
      outputReturn();
    }

    assert(parents->returned);
    // Restore parent function's context
    parents = parents->last;

    // Push Refs
    for (string& var : parents->local_reference_vars_) {
      size_t i = insert(current().local_reference_vars_, var);
      output(Operation::PushReference, i);
    }
    size_t num_local_refs = current().local_reference_vars_.size();
    for (string& var : parents->free_reference_vars_) {
      size_t i = insert(current().free_vars_, var);
      output(Operation::PushReference, num_local_refs + i);
    }

    // Push num_refs
    size_t num_refs = parents->local_reference_vars_.size() + parents->free_reference_vars_.size();
    if (num_refs > 0)
      loadConst(num_refs);

    // Push function
    output(Operation::LoadFunc, current().functions_.size() - 1);

    // Alloc closure if needed
    if (num_refs > 0) {
      output(Operation::AllocClosure);
    }
  }

  void Compiler::visit(Record& rec) {
    output(Operation::AllocRecord);
    rec.record.iterate([this] (string key, Expression *value) {
      output(Operation::Dup);
      transpile(value);
      output(Operation::FieldStore, insert(current().names_, key));
    });
  }

  void Compiler::visit(ValueConstant<bool>& boolconst) {
    loadBool(boolconst.value);
  }

  void Compiler::visit(StringConstant& strconst) {
    loadConst(strconst.value);
  }

  void Compiler::visit(ValueConstant<int>& intconst) {
    loadConst(intconst.value);
  }

  void Compiler::visit(NullConstant& nullconst) {
    loadNone();
  }

  template <BinOpSym op>
  void Compiler::visitBinop(BinaryOp<op>& binop, Operation operation, bool reverse) {
    if (reverse) {
      transpile(binop.right);
      transpile(binop.left);
    } else {
      transpile(binop.left);
      transpile(binop.right);
    }
    output(operation);
  }

  template <UnOpSym op>
  void Compiler::visitUnop(UnaryOp<op>& unop, Operation operation) {
    transpile(unop.expr);
    output(operation);
  }

  void Compiler::visit(BinaryOp<OR>& orop) {
    this->visitBinop(orop, Operation::Or);
  }

  void Compiler::visit(BinaryOp<AND>& andop) {
    this->visitBinop(andop, Operation::And);
  }

  void Compiler::visit(BinaryOp<LT>& ltop) {
    this->visitBinop(ltop, Operation::Gt, true);
  }

  void Compiler::visit(BinaryOp<LTE>& lteop) {
    this->visitBinop(lteop, Operation::Geq, true);
  }

  void Compiler::visit(BinaryOp<GT>& gtop) {
    this->visitBinop(gtop, Operation::Gt);
  }

  void Compiler::visit(BinaryOp<GTE>& gteop) {
    this->visitBinop(gteop, Operation::Geq);
  }

  void Compiler::visit(BinaryOp<EQ>& eqop) {
    this->visitBinop(eqop, Operation::Eq);
  }

  void Compiler::visit(BinaryOp<PLUS>& plusop) {
    this->visitBinop(plusop, Operation::Add);
  }

  void Compiler::visit(BinaryOp<MINUS>& minusop) {
    this->visitBinop(minusop, Operation::Sub);
  }

  void Compiler::visit(BinaryOp<MUL>& mulop) {
    this->visitBinop(mulop, Operation::Mul);
  }

  void Compiler::visit(BinaryOp<DIV>& divop) {
    this->visitBinop(divop, Operation::Div);
  }

  void Compiler::visit(UnaryOp<NOT>& notop) {
    this->visitUnop(notop, Operation::Not);
  }

  void Compiler::visit(UnaryOp<NEG>& negop) {
    this->visitUnop(negop, Operation::Neg);
  }
}
