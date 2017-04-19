#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "../bcparser/parser.h"
#include "../bcparser/lexer.h"
#include "../bccompiler/Compiler.h"
#include "Interpreter.h"
#include <iostream>

using namespace std;

enum Mode {SOURCE, BYTECODE};

void usage() {
  cout << "Usage: vm [-s|-b] filename" << endl;
}

int main(int argc, char** argv)
{
  void* scanner;
  FILE* infile;
  Mode mode;
  shared_ptr<BC::Function> function;

  if (argc == 2) {
    infile = stdin;
  } else if (argc == 3) {
    infile = fopen(argv[2], "r");
    if (!infile) {
      cout << "error: cannot open " << argv[2] << endl;
      return 1;
    }
  } else {
    usage();
    return 1;
  }

  if (strcmp(argv[1], "-s") == 0) {
    mode = SOURCE;
  } else if (strcmp(argv[1], "-b") == 0) {
    mode = BYTECODE;
  } else {
    usage();
    return 1;
  }

  if (mode == SOURCE) {
    yylex_init(&scanner);
    yyset_in(infile, scanner);

    AST::Program* program;

    if (yyparse(scanner, program) != 0) {
      cout << "source parsing failed!" << endl;
      return 1;
    }

    BC::Compiler compiler;
    program->accept(compiler);
    function = compiler.result;

  } else {
    bclex_init(&scanner);
    bcset_in(infile, scanner);

    BC::Function* funcptr;
    if (bcparse(scanner, funcptr) != 0) {
      cout << "bytecode parsing failed!" << endl;
      return 1;
    }
    function = std::shared_ptr<BC::Function>(funcptr);
  }

  VM::Interpreter interpreter(function, 10);
  try {
    return interpreter.interpret();
  } catch (SystemException& ex) {
    cout << ex.what() << endl;
    return 1;
  }

  return 0;
}
