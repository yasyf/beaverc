#include <tuple>
#include <string>
#include <algorithm>
#include <iostream>
#include "Value.h"
#include "NativeFunction.h"
#include "Interpreter.h"
#include "FunctionScanner.h"

using namespace std;

template <typename T>
T* interp_cast(Value *value) {
  T* casted;
  if (!(casted = dynamic_cast<T*>(value))) {
    throw IllegalCastException(value);
  }
  return casted;
}

Value* NativePrint(vector<Value *> args) {
  cout << args[0]->toString() << endl;
  return NoneValue::Singleton();
}

Value* NativeInput(vector<Value *> args) {
  string line;
  getline(cin, line);
  return new StringValue(line);
}

Value* NativeIntcast(vector<Value *> args) {
  StringValue *str = interp_cast<StringValue>(args[0]);
  return new IntegerValue(stoi(str->value));
}

Interpreter::Interpreter() : heap() {
  heap.global->Update("print", new NativeFunction({"s"}, NativePrint));
  heap.global->Update("input", new NativeFunction({}, NativeInput));
  heap.global->Update("intcast", new NativeFunction({"s"}, NativeIntcast));
}

void Interpreter::print(string msg, bool newline = false) {
  cout << msg;
  if (newline)
    cout << endl;
}

void Interpreter::println(string msg) {
  print(msg, true);
}

void Interpreter::exec(AST_node *node, Value *asval = nullptr) {
  asvals.push(asval);
  node->accept(*this);
  asvals.pop();
}

Value* Interpreter::eval(Expression *exp) {
  exec(exp);
  return retval;
}

void Interpreter::ReturnVal(Value *retval) {
  this->retval = retval;
}

void Interpreter::visit(Block& block) {
  for (Statement *stmt : block.statements) {
    exec(stmt);
    if (heap.current()->returned())
      break;
  }
}

void Interpreter::visit(Assignment& assign) {
  Value *asval = eval(assign.expr);
  exec(assign.lhs, asval);
}

void Interpreter::visit(Name& name) {
  if (asvals.top()) {
    heap.UpdateVar(name.name, asvals.top());
  } else {
    ReturnVal(heap.ReadVar(name.name));
  }
}

void Interpreter::visit(FieldDereference& fd) {
  Value *base = eval(fd.base);
  RecordValue *record = interp_cast<RecordValue>(base);
  if (asvals.top()) {
    record->Update(fd.field->name, asvals.top());
  } else {
    ReturnVal(record->Read(fd.field->name));
  }
}

void Interpreter::visit(IndexExpression& ie) {
  Value *base = eval(ie.base);
  RecordValue *record = interp_cast<RecordValue>(base);
  string field = eval(ie.index)->toString();
  if (asvals.top()) {
    record->Update(field, asvals.top());
  } else {
    ReturnVal(record->Read(field));
  }
}

void Interpreter::visit(IfStatement& is) {
  Value *cond = eval(is.cond);
  BooleanValue *boolean = interp_cast<BooleanValue>(cond);
  if (boolean->value) {
    exec(is.thenBlock);
  } else {
    exec(is.elseBlock);
  }
}

void Interpreter::visit(WhileLoop& wl) {
  BooleanValue *boolean;

  boolean = interp_cast<BooleanValue>(eval(wl.cond));
  while (boolean->value && !heap.current()->returned()) {
    exec(wl.body);
    boolean = interp_cast<BooleanValue>(eval(wl.cond));
  }
}

void Interpreter::visit(Return& ret) {
  heap.current()->SetReturn(eval(ret.expr));
}

void Interpreter::visit(ValueConstant<bool>& boolconst) {
  ReturnVal(new BooleanValue(boolconst.value));
}

void Interpreter::visit(ValueConstant<string>& strconst) {
  ReturnVal(new StringValue(strconst.value));
}

void Interpreter::visit(ValueConstant<int>& intconst) {
  ReturnVal(new IntegerValue(intconst.value));
}

void Interpreter::visit(NullConstant& nullconst) {
  ReturnVal(NoneValue::Singleton());
}

void Interpreter::visit(Record& rec) {
  RecordValue *record = new RecordValue();
  for (auto& kv : rec.record) {
    record->Update(kv.first, eval(kv.second));
  }
  ReturnVal(record);
}

void Interpreter::visit(Function& func) {
  vector<string> args;
  transform(func.arguments.begin(), func.arguments.end(), back_inserter(args), [] (Name *n) { return n->name; });
  ReturnVal(new FunctionValue(heap.current(), func.body, args));
}

void Interpreter::visit(Call& call) {
  FunctionValue *func = interp_cast<FunctionValue>(eval(call.target));
  if (call.arguments.size() != func->arguments.size()) {
    throw RuntimeException("invalid number of arguments!");
  }

  if (NativeFunction *nf = dynamic_cast<NativeFunction*>(func)) {
    vector<Value *> args;
    transform(call.arguments.begin(), call.arguments.end(), back_inserter(args), [this] (Expression *e) { return eval(e); });
    ReturnVal(nf->call(args));
    return;
  }

  StackFrame *frame = func->frame->CreateChild();
  FunctionScanner scanner(frame);
  func->code->accept(scanner);

  for (size_t i = 0; i < call.arguments.size(); i++) {
    frame->Update(func->arguments[i], eval(call.arguments[i]));
  }

  heap.push(frame);
  exec(func->code);
  if (!frame->returned())
    frame->SetReturn(NoneValue::Singleton());
  ReturnVal(heap.pop()->GetReturn());
}

void Interpreter::visit(CallStatement& cs) {
  exec(cs.call);
}

void Interpreter::visit(BinaryOp<OR>& orop) {
  BooleanValue *left = interp_cast<BooleanValue>(eval(orop.left));
  BooleanValue *right = interp_cast<BooleanValue>(eval(orop.right));
  ReturnVal(new BooleanValue(left->value || right->value));
}

void Interpreter::visit(BinaryOp<AND>& andop) {
  BooleanValue *left = interp_cast<BooleanValue>(eval(andop.left));
  BooleanValue *right = interp_cast<BooleanValue>(eval(andop.right));
  ReturnVal(new BooleanValue(left->value && right->value));
}

void Interpreter::visit(BinaryOp<LT>& ltop) {
  IntegerValue *left = interp_cast<IntegerValue>(eval(ltop.left));
  IntegerValue *right = interp_cast<IntegerValue>(eval(ltop.right));
  ReturnVal(new BooleanValue(left->value < right->value));
}

void Interpreter::visit(BinaryOp<LTE>& lteop) {
  IntegerValue *left = interp_cast<IntegerValue>(eval(lteop.left));
  IntegerValue *right = interp_cast<IntegerValue>(eval(lteop.right));
  ReturnVal(new BooleanValue(left->value <= right->value));
}

void Interpreter::visit(BinaryOp<GT>& gtop) {
  IntegerValue *left = interp_cast<IntegerValue>(eval(gtop.left));
  IntegerValue *right = interp_cast<IntegerValue>(eval(gtop.right));
  ReturnVal(new BooleanValue(left->value > right->value));
}

void Interpreter::visit(BinaryOp<GTE>& gteop) {
  IntegerValue *left = interp_cast<IntegerValue>(eval(gteop.left));
  IntegerValue *right = interp_cast<IntegerValue>(eval(gteop.right));
  ReturnVal(new BooleanValue(left->value >= right->value));
}

void Interpreter::visit(BinaryOp<EQ>& eqop) {
  ReturnVal(new BooleanValue(eval(eqop.left)->equals(eval(eqop.right))));
}

template<BinOpSym op, typename F>
void Interpreter::visitIntOp(BinaryOp<op>& binop, F func) {
  IntegerValue *left = interp_cast<IntegerValue>(eval(binop.left));
  IntegerValue *right = interp_cast<IntegerValue>(eval(binop.right));
  ReturnVal(new IntegerValue(func(left->value, right->value)));
}

void Interpreter::visit(BinaryOp<PLUS>& plusop) {
  Value *left = eval(plusop.left);
  Value *right = eval(plusop.right);

  if (dynamic_cast<StringValue*>(left) || dynamic_cast<StringValue*>(right)) {
    ReturnVal(new StringValue(left->toString() + right->toString()));
  } else {
    visitIntOp(plusop, [] (int a, int b) { return a + b; });
  }
}

void Interpreter::visit(BinaryOp<MINUS>& minusop) {
  visitIntOp(minusop, [] (int a, int b) { return a - b; });
}

void Interpreter::visit(BinaryOp<MUL>& mulop) {
  visitIntOp(mulop, [] (int a, int b) { return a * b; });
}

void Interpreter::visit(BinaryOp<DIV>& divop) {
  visitIntOp(divop, [] (int a, int b) {
    if (b == 0) {
      throw IllegalArithmeticException();
    }
    return a / b;
  });
}

void Interpreter::visit(UnaryOp<NOT>& notop) {
  BooleanValue *val = interp_cast<BooleanValue>(eval(notop.expr));
  ReturnVal(new BooleanValue(!val->value));
}

void Interpreter::visit(UnaryOp<NEG>& negop) {
  IntegerValue *val = interp_cast<IntegerValue>(eval(negop.expr));
  ReturnVal(new IntegerValue(-val->value));
}

void Interpreter::visit(Program& prog) {
  exec(prog.block);
}

void Interpreter::visit(Global& global) {}
