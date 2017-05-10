#include "include/x64asm.h"
#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "../bcparser/parser.h"
#include "../bcparser/lexer.h"
#include "../bccompiler/Compiler.h"
#include "../ir/OptimizingCompiler.h"
#include "Compiler.h"
#include "PrettyPrinter.h"
#include "BinaryPrinter.h"
#include <iostream>

using namespace std;

enum Mode {SOURCE, BYTECODE};

void usage() {
  cout << "Usage: asm <-s|-b> <filename> <binary|pretty> <indices down the parse tree>" << endl;
}

shared_ptr<BC::Function> getBytecodeFunction(int argc, char** argv) {
  void* scanner;
  FILE* infile;
  Mode mode;

  if (argc == 2) {
    infile = stdin;
  } else if (argc >= 3) {
    infile = fopen(argv[2], "r");
    if (!infile) {
      cout << "error: cannot open " << argv[2] << endl;
      exit(1);
    }
  }

  if (strcmp(argv[1], "-s") == 0) {
    mode = SOURCE;
  } else if (strcmp(argv[1], "-b") == 0) {
    mode = BYTECODE;
  } else {
    usage();
    exit(1);
  }

  shared_ptr<BC::Function> result;

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
    result = compiler.result;

  } else {
    bclex_init(&scanner);
    bcset_in(infile, scanner);

    BC::Function* funcptr;
    if (bcparse(scanner, funcptr) != 0) {
      cout << "bytecode parsing failed!" << endl;
      exit(1);
    }
  }

  int index = 4;

  while (index < argc) {
    result = result->functions_[stoi(argv[index])];
    index++;
  }

  return result;
}

int main(int argc, char** argv)
{
  auto bytecode = getBytecodeFunction(argc, argv);

  IR::InstructionList ir;
  IR::OptimizingCompiler ir_compiler(bytecode, ir);
  size_t temp_count = ir_compiler.compile();

  ASM::Compiler asm_compiler(ir, temp_count);
  x64asm::Function assm;
  asm_compiler.compileInto(assm);

  if (strcmp("binary", argv[3]) == 0) {
    ASM::BinaryPrinter printer(assm);
    printer.print();
  } else {
    ASM::PrettyPrinter printer(assm);
    printer.print();
  }

  return 0;
}
