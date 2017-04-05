#pragma once
#include "Op.h"

namespace AST {
  class Program;
  class Block;
  class Name;
  class IndexExpression;
  class FieldDereference;
  class Assignment;
  class Call;
  class CallStatement;
  class Global;
  class IfStatement;
  class WhileLoop;
  class Return;
  class Function;
  class Record;
  class NullConstant;
  template <class T>
  class ValueConstant;
  class StringConstant;
  template <BinOpSym op>
  class BinaryOp;
  template <UnOpSym op>
  class UnaryOp;
}
