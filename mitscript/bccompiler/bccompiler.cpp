#include <iostream>
#include "../parser/parser.h"
#include "../parser/lexer.h"
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

  FILE* infile;

  if (argc == 1) {
    infile = stdin;
  } else if (argc == 2) {
    infile = fopen(argv[1], "r");
    if (!infile) {
      cout << "error: cannot open " << argv[1] << endl;
      return 1;
    }
  } else {
    return 1;
  }

  yylex_init(&scanner);
  yyset_in(infile, scanner);

  if (yyparse(scanner, program) != 0) {
    cout << "Parsing failed" << endl;
    return 1;
  }

  program->accept(transpiler);
  printer.print(*transpiler.result, cout);
}
