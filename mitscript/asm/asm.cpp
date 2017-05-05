#include "include/x64asm.h"
#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "../bcparser/parser.h"
#include "../bcparser/lexer.h"
#include "../bccompiler/Compiler.h"
#include "../ir/Compiler.h"
#include "Compiler.h"
#include "PrettyPrinter.h"
#include <iostream>

using namespace std;

enum Mode {SOURCE, BYTECODE};

void usage() {
  cout << "Usage: ir <-s|-b> <filename>" << endl;
}

shared_ptr<BC::Function> getBytecodeFunction(int argc, char** argv) {
  void* scanner;
  FILE* infile;
  Mode mode;

  if (argc == 2) {
    infile = stdin;
  } else if (argc == 3) {
    infile = fopen(argv[2], "r");
    if (!infile) {
      cout << "error: cannot open " << argv[2] << endl;
      exit(1);
    }
  } else {
    usage();
    exit(1);
  }

  if (strcmp(argv[1], "-s") == 0) {
    mode = SOURCE;
  } else if (strcmp(argv[1], "-b") == 0) {
    mode = BYTECODE;
  } else {
    usage();
    exit(1);
  }

  if (mode == SOURCE) {
    yylex_init(&scanner);
    yyset_in(infile, scanner);

    AST::Program* program;

    if (yyparse(scanner, program) != 0) {
      cout << "source parsing failed!" << endl;
      exit(1);
    }

    BC::Compiler compiler;
    program->accept(compiler);
    return compiler.result;

  } else {
    bclex_init(&scanner);
    bcset_in(infile, scanner);

    BC::Function* funcptr;
    if (bcparse(scanner, funcptr) != 0) {
      cout << "bytecode parsing failed!" << endl;
      exit(1);
    }
    return shared_ptr<BC::Function>(funcptr);
  }
}

int main(int argc, char** argv)
{
  auto bytecode = getBytecodeFunction(argc, argv);

  IR::Compiler ir_compiler(bytecode);
  auto ir = ir_compiler.compile();

  GC::CollectedHeap heap(0);
  VM::ClosureFunctionValue closure(heap, bytecode);

  ASM::Compiler asm_compiler(ir, closure);
  auto assm = asm_compiler.compile();

  ASM::PrettyPrinter printer(assm);
  printer.print();

  return 0;
}
