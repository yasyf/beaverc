#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "../bcparser/parser.h"
#include "../bcparser/lexer.h"
#include "../bccompiler/Compiler.h"
#include "Interpreter.h"
#include "globals.h"
#include "mem.h"
#include <iostream>

using namespace std;

#define MB_TO_B 1024 * 1024
#define KB_TO_B 1024

enum Mode {SOURCE, BYTECODE};

void usage() {
  cout << "Usage: vm -mem <MEM> <-s|-b> <filename>" << endl;
}

int main(int argc, char** argv)
{
  void* scanner;
  FILE* infile;
  Mode mode;
  shared_ptr<BC::Function> function;

  if (argc == 4) {
    infile = stdin;
  } else if (argc == 5) {
    infile = fopen(argv[4], "r");
    if (!infile) {
      cout << "error: cannot open " << argv[4] << endl;
      return 1;
    }
  } else {
    usage();
    return 1;
  }

  if (strcmp(argv[1], "-mem") != 0) {
    usage();
    return 1;
  }

  if (strcmp(argv[3], "-s") == 0) {
    mode = SOURCE;
  } else if (strcmp(argv[3], "-b") == 0) {
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

  size_t max_memory = std::stoi(argv[2]) * MB_TO_B;
  size_t current_memory = rss();
  size_t usable_memory = (max_memory > current_memory) ? max_memory - current_memory : 0;

  #ifdef DEBUG
    cout
      << "Starting with "
      << current_memory / KB_TO_B
      << " kb used, "
      << usable_memory / KB_TO_B
      << " kb usable."
      << endl
    ;
  #endif

  interpreter = new VM::Interpreter(function, usable_memory);
  try {
    return interpreter->interpret();
  } catch (SystemException& ex) {
    cout << ex.what() << endl;
    return 1;
  }

  return 0;
}
