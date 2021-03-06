#include "../options.h"
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
#include <sys/time.h>
#include <sys/resource.h>

using namespace std;

#define MB_TO_B 1024 * 1024
#define KB_TO_B 1024

enum Mode {SOURCE, BYTECODE};

int main(int argc, char** argv)
{
  bool print_memory = false;
  void* scanner;
  FILE* infile;
  Mode mode = SOURCE;
  shared_ptr<BC::Function> function;

  const char* max_mem = "500";

  while (true) {
    static struct option long_options[] =
      {
        /* These options don’t set a flag.
           We distinguish them by their indices. */
        {"mem",               required_argument, 0, 'm'},
        {"source",            no_argument,       0, 's'},
        {"bytecode",          no_argument,       0, 'b'},
        {"opt",               required_argument, 0, 'o'},
        {"memory-usage",      no_argument,       0, 'u'},
        {"memory-trace",      no_argument,       0, 't'},
        {"compile-errors",    no_argument,       0, 'e'},
        {0, 0, 0, 0}
      };
    int OPTIMIZATION_index = 0;
    int c = getopt_long (argc, argv, "m:sbo:",
                     long_options, &OPTIMIZATION_index);

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
          set_optimization(OPTIMIZATION_MACHINE_CODE);
        } else if (strcmp(optarg, "string-trees") == 0) {
          set_optimization(OPTIMIZATION_STRING_TREES);
        } else if (strcmp(optarg, "compile-only") == 0) {
          set_optimization(OPTIMIZATION_MACHINE_CODE);
          set_optimization(OPTIMIZATION_COMPILE_ONLY);
        } else if (strcmp(optarg, "gc-generational") == 0) {
          set_optimization(OPTIMIZATION_GC_GENERATIONAL);
        } else if (strcmp(optarg, "all") == 0) {
          set_optimization(OPTIMIZATION_MACHINE_CODE);
          set_optimization(OPTIMIZATION_GC_GENERATIONAL);
          set_optimization(OPTIMIZATION_OPTIMIZATION_PASSES);
        }
        break;
      case 'u':
        set_option(OPTION_SHOW_MEMORY_USAGE);
        break;
      case 't':
        set_option(OPTION_SHOW_MEMORY_TRACE);
        break;
      case 'e':
        set_option(OPTION_COMPILE_ERRORS);
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

  if (has_option(OPTION_SHOW_MEMORY_USAGE)) {
    cerr << current_memory / KB_TO_B << " kb used for setup." << endl;
    cerr << usable_memory / KB_TO_B << " kb usable during program life" << endl;
  }

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

  std::set_terminate([](){
    try {
      std::exception_ptr eptr = std::current_exception();
      if (eptr) {
        std::rethrow_exception(eptr);
      }
    } catch (SystemException& ex) {
      cout << ex.what() << endl;
      exit(1);
    }
  });

  // cout << "Value: " << sizeof(VM::Value) << endl;
  // cout << "ReferenceValue: " << sizeof(VM::ReferenceValue) << endl;
  // cout << "RecordValue: " << sizeof(VM::RecordValue) << endl;
  // cout << "ClosureFunctionValue: " << sizeof(VM::ClosureFunctionValue) << endl;
  // cout << "BareFunctionValue: " << sizeof(VM::BareFunctionValue) << endl;
  // cout << "Unordered map: " << sizeof(std::unordered_map<const char*, VM::Value>) << endl;
  // cout << "Vector: " << sizeof(std::vector<const char*>) << endl;

  int result = interpreter->interpret();
  if (has_option(OPTION_SHOW_MEMORY_USAGE)) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    cerr << interpreter->heap.max_bytes_used / KB_TO_B << " kb predicted used by allocs" << endl;
    cerr << usage.ru_maxrss - (current_memory / KB_TO_B) << " kb actually used by allocs" << endl;
    cerr << interpreter->heap.fast_collections << " fast collections" << endl;
    cerr << interpreter->heap.successful_fast_collections << " successful" << endl;
    cerr << interpreter->heap.full_collections << " full collections" << endl;
    cerr << interpreter->heap.successful_full_collections << " successful" << endl;
    cerr << usage.ru_maxrss << " kb actually used." << endl;
  }
  return result;
}
