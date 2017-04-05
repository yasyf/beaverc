#include <iostream>
#include "../parser.h"
#include "../lexer.h"
#include "../AST.h"
#include "Types.h"
#include "Transpiler.h"
#include "PrettyPrinter.h"

using namespace std;

int main(int argc, char** argv){
  BC::PrettyPrinter printer;
  BC::Transpiler transpiler;
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
