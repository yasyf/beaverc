#include <iostream>
#include "../parser.h"
#include "../lexer.h"
#include "../AST.h"
#include "Types.h"
#include "Compiler.h"
#include "PrettyPrinter.h"

using namespace std;

int main(int argc, char** argv){
  BC::PrettyPrinter printer;
  BC::Compiler transpiler;
  Program* program;
  void* scanner;

  yylex_init(&scanner);
  yyset_in(stdin, scanner);

  if (yyparse(scanner, program) != 0) {
    cout << "Parsing failed" << endl;
    return 1;
  }

  program->accept(transpiler);
  printer.print(*transpiler.result, cout);
}
