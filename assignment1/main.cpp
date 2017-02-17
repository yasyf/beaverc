
#include "parser.h"
#include "lexer.h"
#include "AST.h"
#include "PrettyPrinter.h"
#include <iostream>

using namespace std;

#define TRACE 1


int main(int argc, char** argv){
  void* scanner;
  yylex_init(&scanner);
  yyset_in(stdin, scanner);

  #if TRACE
    yydebug = 1;
    yyset_debug(1, scanner);
  #endif

  Statement* output;
  int rvalue = yyparse(scanner, output);
  if(rvalue == 1){
	cout<<"Parsing failed"<<endl;
	return 1;
  }
  // PrettyPrinter printer;
  // output->accept(printer);
}
