CC = g++
CXX = g++

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = g++-6
    CXX = g++-6
endif

LEX = flex
YACC = bison

CFLAGS = -std=c++14 -I../x64asm -I../udis86/libudis86 -Wl,--gc-sections
CXXFLAGS = -std=c++14 -I../x64asm -I../udis86/libudis86 -MMD -MP -Wl,--gc-sections
LDFLAGS = -lstdc++ -L../x64asm/lib -L../udis86/libudis86/.libs -Wl,--gc-sections
LIBS = -lx64asm -ludis86

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g -O0
    CXXFLAGS += -DDEBUG -g -O0
else
    CFLAGS += -DNDEBUG -O2
    CXXFLAGS += -DNDEBUG -O2
endif

LABELS ?= 0
ifeq ($(LABELS), 1)
    CFLAGS += -g
    CXXFLAGS += -g
endif

PROFILE ?= 0
ifeq ($(PROFILE), 1)
    CFLAGS += -pg
    CXXFLAGS += -pg
endif

CPPSOURCES = $(wildcard *.cpp)
CSOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.cpp, %.o, $(CPPSOURCES)) $(patsubst %.c, %.o, $(CSOURCES))

PARSE_OBJECTS = parser/parser.o parser/lexer.o
BCPARSE_OBJECTS = bcparser/parser.o bcparser/lexer.o

.PHONY: parser bcparser clean extraclean irclean asmclean all

default: vm bccompiler interpreter pprinter ir

BIOBJECTS = $(patsubst %.cpp, %.o, $(wildcard interpreter/*.cpp))
interpreter: parser $(PARSE_OBJECTS) $(OBJECTS) $(BIOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(BIOBJECTS) -o bin/$@

COOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/*.cpp))
bccompiler: parser $(PARSE_OBJECTS) $(OBJECTS) $(COOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(COOBJECTS) -o bin/$@

BCCOOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/Compiler*.cpp))
BCCCOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccchecker/*.cpp))
bccchecker: parser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(BCCCOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(BCPARSE_OBJECTS) $(PARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(BCCCOBJECTS) -o bin/$@

PPOBJECTS = $(patsubst %.cpp, %.o, $(wildcard pprinter/*.cpp))
pprinter: parser $(PARSE_OBJECTS) $(OBJECTS) $(PPOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(PPOBJECTS) -o bin/$@

ASOBJECTS = $(patsubst %.cpp, %.o, $(wildcard astprinter/*.cpp))
PPASOBJECTS = pprinter/PrettyPrinter.o
astprinter: parser $(PARSE_OBJECTS) $(OBJECTS) pprinter/PrettyPrinter.o $(ASOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(PPASOBJECTS) $(ASOBJECTS) -o bin/$@

GCOBJECTS = $(patsubst %.cpp, %.o, $(wildcard gc/*.cpp))
IROBJECTS = $(patsubst %.cpp, %.o, $(wildcard ir/*.cpp))
OPTSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard vm/option*.cpp))
ir: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(IROBJECTS) $(GCOBJECTS) $(OPTSOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(IROBJECTS) $(GCOBJECTS) $(OPTSOBJECTS) $(LIBS) -o bin/$@

ASMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard asm/*.cpp))
IRVMOBJECTS = $(patsubst %.cpp, %.o, $(filter-out ir/ir.cpp,$(wildcard ir/*.cpp)))
VMASMOBJECTS = $(patsubst %.cpp, %.o, $(filter-out vm/vm.cpp,$(wildcard vm/*.cpp)))
asm: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(VMASMOBJECTS) $(GCOBJECTS) $(ASMOBJECTS) $(IRVMOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(VMASMOBJECTS) $(GCOBJECTS) $(ASMOBJECTS) $(IRVMOBJECTS) $(LIBS) -o bin/$@

VMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard vm/*.cpp))
ASMVMOBJECTS = $(patsubst %.cpp, %.o, $(filter-out asm/asm.cpp,$(wildcard asm/*.cpp)))
vm: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(VMOBJECTS) $(GCOBJECTS) $(ASMVMOBJECTS) $(IRVMOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(VMOBJECTS) $(GCOBJECTS) $(ASMVMOBJECTS) $(IRVMOBJECTS) $(LIBS) -o bin/$@

CGOBJECTS = $(patsubst %.cpp, %.o, $(wildcard codegen/*.cpp))
codegen: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(CGOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(CGOBJECTS) $(LIBS) -o bin/$@

parser: parser/lexer.c parser/parser.cpp

parser/lexer.c:
	$(LEX) --header-file=parser/lexer.h -o parser/lexer.c parser/lexer.lex

parser/parser.cpp:
	$(YACC) --defines=parser/parser.h -o parser/parser.cpp parser/parser.yy

bcparser: bcparser/lexer.c bcparser/parser.cpp

bcparser/lexer.c:
	$(LEX) --header-file=bcparser/lexer.h -o bcparser/lexer.c bcparser/lexer.lex

bcparser/parser.cpp:
	$(YACC) --defines=bcparser/parser.h -o bcparser/parser.cpp bcparser/parser.yy

-include $(shell find -name '*.d')

%.o: %.cpp %.c %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f bin/* *.o */*.o *.d */*.d

irclean:
	rm -rf bin/ir ir/*.o

asmclean:
	rm -rf bin/vm bin/asm bin/ir vm/*.o asm/*.o ir/*.o

extraclean: clean
	rm -f parser/*.c parser/*.cpp parser/*.h bcparser/*.c bcparser/*.cpp bcparser/*.h
