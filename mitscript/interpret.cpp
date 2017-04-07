#include <iostream>
#include "parser.h"
#include "lexer.h"
#include "Interpreter.h"
#include "Exception.h"

using namespace std;

int main(int argc, char** argv){
  Program* program;
  void* scanner;

  if (argc < 2) {
    cout << "missing file name" << endl;
    return 1;
  }

  FILE *infile = fopen(argv[1], "r");
  if (infile == NULL) {
    cout << "cannot open file " << argv[1] << endl;
    return 1;
  }

  yylex_init(&scanner);
  yyset_in(infile, scanner);

  if(yyparse(scanner, program) != 0){
    cout << "Parsing failed" << endl;
    return 1;
  }

  try {
    Interpreter interpreter;
    program->accept(interpreter);
  } catch(InterpreterException &ex) {
    cout << ex.what() << endl;
    return 1;
  }

  return 0;
}
