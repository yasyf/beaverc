#include "../parser/parser.h"
#include "../parser/lexer.h"
#include "../bcparser/parser.h"
#include "../bcparser/lexer.h"
#include "../bccompiler/Compiler.h"
#include "Interpreter.h"
#include "globals.h"
#include "mem.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

using namespace std;

#define MB_TO_B 1024 * 1024
#define KB_TO_B 1024

enum Mode {SOURCE, BYTECODE};

int main(int argc, char** argv)
{
  void* scanner;
  FILE* infile;
  Mode mode = SOURCE;
  shared_ptr<BC::Function> function;

  const char* max_mem = "4";

  while (true) {
    static struct option long_options[] =
      {
        /* These options don’t set a flag.
           We distinguish them by their indices. */
        {"mem",      required_argument, 0, 'm'},
        {"source",   no_argument,       0, 's'},
        {"bytecode", no_argument,       0, 'b'},
        {"opt",      required_argument, 0, 'o'},
        {0, 0, 0, 0}
      };
    int option_index = 0;
    int c = getopt_long (argc, argv, "m:sbo:",
                     long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
      case 'm':
        max_mem = optarg;
        break;
      case 's':
        mode = SOURCE;
        break;
      case 'b':
        mode = BYTECODE;
        break;
      case 'o':
        if (strcmp(optarg, "machine-code-only") == 0) {
          set_option(OPTION_MACHINE_CODE_ONLY);
        }
        if (strcmp(optarg, "string-trees") == 0) {
          set_option(OPTION_STRING_TREES);
        }
        if (strcmp(optarg, "compile-only") == 0) {
          set_option(OPTION_COMPILE_ONLY);
        }
        break;
      case '?':
        break;
      default:
        abort();
    }
  }

  if (optind < argc) {
    infile = fopen(argv[optind], "r");
    if (!infile) {
      cout << "error: cannot open " << argv[optind] << endl;
      return 1;
    }
  } else {
    infile = stdin;
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

  size_t max_memory = std::stoi(max_mem) * MB_TO_B;
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
