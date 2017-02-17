
#include "parser.h"
#include "lexer.h"
#include "AST.h"
#include "PrettyPrinter.h"
#include <iostream>

using namespace std;

#define TRACE_PARSE 1


int main(int argc, char** argv){
  #if TRACE_PARSE
    yydebug = 1;
  #endif

  void* scanner;
  yylex_init(&scanner);
  yyset_in(stdin, scanner);
  Statement* output;
  int rvalue = yyparse(scanner, output);
  if(rvalue == 1){
	cout<<"Parsing failed"<<endl;
	return 1;
  }
  // PrettyPrinter printer;
  // output->accept(printer);
}
