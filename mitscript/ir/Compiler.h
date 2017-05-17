#pragma once
#include "../bccompiler/Instructions.h"
#include "Instructions.h"
#include "Exception.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <stack>
#include <map>

using namespace std;

namespace IR {
  class Compiler {
  public:
    stack<shared_ptr<Temp>> operands;
    shared_ptr<BC::Function> bytecode;
    InstructionList& instructions;
    vector<shared_ptr<Temp>> temps;
    map<size_t, shared_ptr<Var>> vars;

    shared_ptr<Temp> extraTemp();

  private:
    shared_ptr<Temp> popTemp();

    void swapTemp();

    shared_ptr<Temp> nextTemp();

    template<typename T>
    shared_ptr<T> get_operand(size_t num);

    template<typename T>
    shared_ptr<T> get_singleton();

    template<typename T>
    shared_ptr<Temp> assign(shared_ptr<T> t);

    template<typename T>
    void store(shared_ptr<T> t);

    template<typename T, Assert A>
    T* helper_binop();

    template<typename T, Assert A>
    T* helper_unop();

    template<typename T>
    void int_binop();

    template<typename T>
    void int_to_bool_binop();

    template<typename T>
    void bool_binop();

    template<typename T>
    void int_unop();

    template<typename T>
    void bool_unop();

    void compile(BC::Function& func);

  public:
    Compiler(shared_ptr<BC::Function> bytecode, InstructionList& instructions);
    size_t compile();
  };
}
