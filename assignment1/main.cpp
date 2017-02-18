#include "parser.h"
#include "lexer.h"
#include "AST.h"
#include "PrettyPrinter.h"
#include <iostream>

using namespace std;

#define TRACE 0


int main(int argc, char** argv){
  PrettyPrinter printer;
  Program* program;
  void* scanner;

  yylex_init(&scanner);
  yyset_in(stdin, scanner);

  #if TRACE
    yydebug = 1;
    yyset_debug(1, scanner);
  #endif

  if(yyparse(scanner, program) != 0){
    cout << "Parsing failed" << endl;
    return 1;
  }

  program->accept(printer);
}
