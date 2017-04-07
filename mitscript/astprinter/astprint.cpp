#include <iostream>
#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "../AST.h"
#include "ASTPrinter.h"

using namespace std;

#define TRACE 0


int main(int argc, char** argv){
  ASTPrinter printer;
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
