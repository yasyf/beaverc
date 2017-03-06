#include <tuple>
#include <string>
#include "Interpreter.h"

using namespace std;

template <typename T>
T* interp_cast(Value *value) {
  if (!(T *casted = dynamic_cast<T*>(value))) {
    throw IllegalCastException(value);
  }
  return casted;
}

Value* NativePrint(vector<Value *> args) {
  cout << args[0]->toString() << endl;
  return &NoneSingleton;
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


Value* Interpreter::Return(Value *retval) {
  this->retval = retval;
}

void Interpreter::assign(LHS *lhs, Value *asval) {
  exec(lhs, asval);
}

void Interpreter::visit(Block& block) {
  for (Statement *stmt : block.statements) {
    exec(stmt);
  }
}

void Interpreter::visit(Assignment& assign) {
  Value *asval = eval(assign.expr);
  assign(assign.lhs, asval);
}

void Interpreter::visit(Name& name) {
  if (asvals.top()) {
    heap.UpdateVar(name.name, asvals.top());
  } else {
    Return(heap.ReadVar(name.name));
  }
}

void Interpreter::visit(FieldDereference& fd) {
  Value *base = eval(fd.base);
  RecordValue *record = interp_cast<RecordValue>(base);
  if (asvals.top()) {
    record->Update(fd.field.name, asvals.top());
  } else {
    Return(record->Read(fd.field.name));
  }
}

void Interpreter::visit(IndexExpression& ie) {
  Value *base = eval(ie.base);
  RecordValue *record = interp_cast<RecordValue>(base);
  string field = eval(ie.index).toString();
  if (asvals.top()) {
    record->Update(field, asvals.top());
  } else {
    Return(record->Read(field));
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
  while (boolean->value) {
    exec(wl.body);
    boolean = interp_cast<BooleanValue>(eval(wl.cond));
  }
}

void Interpreter::visit(Return& ret) {
  heap.current().SetReturn(eval(ret.expr));
}

void Interpreter::visit(ValueConstant<bool>& boolconst) {
  Return(new BooleanValue(boolconst.value));
}

void Interpreter::visit(ValueConstant<string>& strconst) {
  Return(new StringValue(strconst.value));
}

void Interpreter::visit(ValueConstant<int>& intconst) {
  Return(new IntegerValue(intconst.value));
}

void Interpreter::visit(NullConstant& nullconst) {
  Return(&NoneSingleton);
}

void Interpreter::visit(Record& rec) {
  RecordValue *record = new RecordValue();
  for (auto& kv : rec.record) {
    record->Update(kv.first.name, eval(kv.second));
  }
  Return(record);
}

void Interpreter::visit(Function& func) {
  Return(new FunctionValue(heap.current(), func.body, func.arguments));
}

void Interpreter::visit(Call& call) {
  FunctionValue *func = interp_cast<FunctionValue>(eval(call.target));
  if (call.arguments.size() != func.arguments.size()) {
    throw RuntimeException("invalid number of arguments!");
  }

  if (NativeFunction *nf = dynamic_cast<NativeFunction*>(func)) {
    Return(nf.call(call.arguments));
    return;
  }

  StackFrame *frame = func->frame->CreateChild();
  FunctionScanner scanner(frame);
  func.code->accept(scanner);

  for (size_t i = 0; i < call.arguments.size(); i++) {
    frame.Update(func.arguments[i], eval(call.arguments[i]));
  }

  frame.SetReturn(&NoneSingleton);

  heap.push(frame);
  exec(func.code);
  Return(heap.pop().GetReturn());
}

void Interpreter::visit(CallStatement& cs) {
  exec(cs.call);
}

void Interpreter::visit(BinaryOp<OR>& orop) {
  BooleanValue *left = interp_cast<BooleanValue>(eval(orop.left));
  BooleanValue *right = interp_cast<BooleanValue>(eval(orop.right));
  Return(left->value || right->value);
}

void PrettyPrinter::visit(BinaryOp<AND>& andop) {
  BooleanValue *left = interp_cast<BooleanValue>(eval(andop.left));
  BooleanValue *right = interp_cast<BooleanValue>(eval(andop.right));
  Return(left->value && right->value);
}

void PrettyPrinter::visit(BinaryOp<LT>& ltop) {
  BooleanValue *left = interp_cast<IntegerValue>(eval(ltop.left));
  BooleanValue *right = interp_cast<IntegerValue>(eval(ltop.right));
  Return(left->value < right->value);
}

void PrettyPrinter::visit(BinaryOp<LTE>& lteop) {
  BooleanValue *left = interp_cast<IntegerValue>(eval(lteop.left));
  BooleanValue *right = interp_cast<IntegerValue>(eval(lteop.right));
  Return(left->value <= right->value);
}

void PrettyPrinter::visit(BinaryOp<GT>& gtop) {
  BooleanValue *left = interp_cast<IntegerValue>(eval(gtop.left));
  BooleanValue *right = interp_cast<IntegerValue>(eval(gtop.right));
  Return(left->value > right->value);
}

void PrettyPrinter::visit(BinaryOp<GTE>& gteop) {
  BooleanValue *left = interp_cast<IntegerValue>(eval(gteop.left));
  BooleanValue *right = interp_cast<IntegerValue>(eval(gteop.right));
  Return(left->value >= right->value);
}

void PrettyPrinter::visit(BinaryOp<EQ>& eqop) {
  Return(eval(eqop.left).equals(eval(eqop.right)));
}

template<BinOpSym op, typename F>
void visitIntOp(BinaryOp<op>& binop, F &func) {
  IntegerValue *left = interp_cast<IntegerValue>(eval(binop.left));
  IntegerValue *right = interp_cast<IntegerValue>(eval(binop.right));
  Return(func(left->value, right->value));
}

void PrettyPrinter::visit(BinaryOp<PLUS>& plusop) {
  Value *left = eval(plusop.left);
  Value *right = eval(plusop.right);

  if (dynamic_cast<StringValue*>(left) || dynamic_cast<StringValue*>(right)) {
    Return(left.toString() + right.toString());
  } else {
    visitIntOp(plusop, [] (int a, int b) { return a + b; });
  }
}

void PrettyPrinter::visit(BinaryOp<MINUS>& minusop) {
  visitIntOp(minusop, [] (int a, int b) { return a - b; });
}

void PrettyPrinter::visit(BinaryOp<MUL>& mulop) {
  visitIntOp(mulop, [] (int a, int b) { return a * b; });
}

void PrettyPrinter::visit(BinaryOp<DIV>& divop) {
  visitIntOp(mulop, [] (int a, int b) {
    if (b == 0) {
      throw IllegalArithmeticException();
    }
    return a / b;
  });
}

void PrettyPrinter::visit(UnaryOp<NOT>& notop) {
  BooleanValue *val = interp_cast<BooleanValue>(eval(notop.expr));
  Return(new BooleanValue(!val->value));
}

void PrettyPrinter::visit(UnaryOp<NEG>& negop) {
  IntegerValue *val = interp_cast<IntegerValue>(eval(negop.expr));
  Return(new IntegerValue(-val->value));
}

