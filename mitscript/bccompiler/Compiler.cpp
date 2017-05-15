#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include <vector>
#include <experimental/optional>
#include "Util.h"
#include "Compiler.h"
#include "Exception.h"
#include "CompilerGlobalsScanner.h"
#include "CompilerLocalsScanner.h"
#include "CompilerRefsScanner.h"
#include "CompilerDependenciesScanner.h"
#include "CompilerErrorsScanner.h"

using namespace std;
using namespace std::experimental;
using namespace AST;

namespace BC {
  const vector<pair<string, int>> Builtins = {{"print", 1}, {"input", 0}, {"intcast", 1}};

  Compiler::Compiler() {
    result = shared_ptr<Function>(new Function());
    result->parameter_count_ = 0;

    root = shared_ptr<FunctionLinkedList>(new FunctionLinkedList(result));
    parents = root;

    for (auto const& b : Builtins) {
      addNativeFunction(b.first, b.second);
    }
  }

  void Compiler::addNativeFunction(string name, size_t argc) {
    static size_t n_native_funcs = 1;
    output(Operation::LoadFunc, -(n_native_funcs++));
    insert(parents->globals_, name);
    output(Operation::StoreGlobal, insert(current().names_, name));
  }

  void Compiler::transpile(AST_node *node, bool storing) {
    bool old_storing = this->storing;
    this->storing = storing;
    node->accept(*this);
    this->storing = old_storing;
  }

  void Compiler::store(AST_node *node) {
    transpile(node, true);
  }

  size_t Compiler::count() {
    return parents->getOut()->size();
  }

  void Compiler::output(Instruction inst) {
    parents->getOut()->push_back(inst);
  }

  void Compiler::output(const Operation operation) {
    output(Instruction(operation, nullopt));
  }

  void Compiler::output(const Operation operation, int32_t operand0) {
    output(Instruction(operation, operand0));
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

  void Compiler::outputLabel(size_t label) {
    current().labels[label] = count();
    output(Operation::Label, label);
  }

  size_t Compiler::reserveLabel() {
    static size_t label_num = 0;
    return label_num++;
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
    if (index(parents->globals_, name.name)) {
      auto i = index(current().names_, name.name);
      if (storing)
        output(Operation::StoreGlobal, *i);
      else
        output(Operation::LoadGlobal, *i);
    } else if (auto i = index(current().free_vars_, name.name)) {
      size_t index = current().local_reference_vars_.size() + *i;
      if (storing) {
        output(Operation::StoreReference, index);
      } else {
        output(Operation::LoadReference, index);
      }
    } else if (auto i = index(current().local_reference_vars_, name.name)) {
      if (storing) {
        output(Operation::StoreReference, *i);
      } else {
        output(Operation::LoadReference, *i);
      }
    } else if (auto i = index(current().local_vars_, name.name)) {
      if (storing)
        output(Operation::StoreLocal, *i);
      else
        output(Operation::LoadLocal, *i);
    } else {
      if (storing && isGlobal()) {
        insert(parents->globals_, name.name);
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
    for (auto arg : call.arguments) {
      transpile(arg);
    }
    transpile(call.target);
    output(Operation::Call, call.arguments.size());
  }

  void Compiler::visit(CallStatement& cs) {
    transpile(cs.call);
    output(Operation::Pop);
    output(Operation::GarbageCollect);
  }

  void Compiler::visit(Global& global) {
    // noop
  }

  void Compiler::visit(IfStatement& is) {
    size_t then_label = reserveLabel();
    size_t else_label = reserveLabel();
    size_t skip_else_label = reserveLabel();

    // Execute if
    transpile(is.cond);
    output(Operation::If, then_label); // skip past else-goto
    output(Operation::Goto, else_label); // else-goto: skip past then-block and if-goto
    outputLabel(then_label);
    transpile(is.thenBlock); // then-block
    output(Operation::Goto, skip_else_label); // if-goto: skip past else-block
    outputLabel(else_label);
    transpile(is.elseBlock); // else-block
    outputLabel(skip_else_label);
  }

  void Compiler::visit(WhileLoop& wl) {
    size_t loop_start_label = reserveLabel();
    size_t body_label = reserveLabel();
    size_t skip_body_label = reserveLabel();

    outputLabel(loop_start_label);
    output(Operation::GarbageCollect);
    transpile(wl.cond); // cond-block
    output(Operation::If, body_label); // skip past end-goto
    output(Operation::Goto, skip_body_label); // end-goto: skip past body-block and loop-goto
    outputLabel(body_label);
    transpile(wl.body); // body-block
    output(Operation::Goto, loop_start_label); // loop-goto: back up to cond-block
    outputLabel(skip_body_label);
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
    parents = parents->extend(function);

    // Fill new function metadata
    CompilerGlobalsScanner globalsScanner;
    globalsScanner.scan(func.body);
    vector<string> globals = globalsScanner.getGlobals();
    for (string& global : globals) {
      insert(parents->globals_, global);
      insert(function->names_, global);
    }

    CompilerLocalsScanner localsScanner(globalsScanner.globals);
    localsScanner.scan(func.body);
    vector<string> locals = localsScanner.getLocals();
    for (string& local : locals)
      insert(function->local_vars_, local);

    CompilerRefsScanner localRefsScanner(function->local_vars_);
    localRefsScanner.scan(func.body);
    for (string& localRef : localRefsScanner.getRefs())
      insert(function->local_reference_vars_, localRef);

    CompilerRefsScanner localRef0sScanner(last().local_reference_vars_, locals, globals);
    localRef0sScanner.scan(func.body);
    for (string& var : localRef0sScanner.getRefs())
      insert(function->free_vars_, var);

    CompilerRefsScanner freeRef0sScanner(parent()->free_reference_vars_, locals, globals);
    freeRef0sScanner.scan(func.body);
    for (string& var : freeRef0sScanner.getRefs())
      insert(function->free_vars_, var);

    CompilerRefsScanner freeRefsScanner(function->free_vars_);
    freeRefsScanner.scan(func.body);
    for (string& freeRef : freeRefsScanner.getRefs())
      insert(parents->free_reference_vars_, freeRef);

    // Fix tiny edge cases
    CompilerDependenciesScanner dependenciesScanner(parents);
    dependenciesScanner.scan(func.body);

    // Transpile new function
    transpile(func.body);

    // Default return
    if (!parents->returned || parents->getOut()->back().operation == Operation::Label) {
      loadNone();
      outputReturn();
    }

    // Check for errors
    CompilerErrorsScanner errorsScanner(function);
    errorsScanner.scan(func.body);

    assert(parents->returned);

    // Restore parent function's context
    parents = parent();

    vector<string> localRefs = localRef0sScanner.getRefs();
    vector<string> freeRefs = freeRef0sScanner.getRefs();
    size_t num_local_ref_vars = current().local_reference_vars_.size();
    for (int j = freeRefs.size(); j > 0; j--) { // Push everything backwards
      string& var = freeRefs[j-1];
      size_t i = index(current().free_vars_, var).value();
      output(Operation::PushReference, num_local_ref_vars + i);
    }

    for (int j = localRefs.size(); j > 0; j--) {
      string& var = localRefs[j-1];
      size_t i = index(current().local_reference_vars_, var).value();
      output(Operation::PushReference, i);
    }

    // Push function
    output(Operation::LoadFunc, current().functions_.size() - 1);

    // Alloc closure
    size_t num_refs = localRef0sScanner.refs.size() + freeRef0sScanner.refs.size();
    output(Operation::AllocClosure, num_refs);
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
