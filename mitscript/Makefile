CC = g++
CXX = g++

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = g++-6
    CXX = g++-6
endif

LEX = flex
YACC = bison

CFLAGS = -std=c++1y
CXXFLAGS = -std=c++1y -I../x64asm
LDFLAGS = -lstdc++ -L../x64asm/lib
LIBS = -lx64asm

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g -O0
    CXXFLAGS += -DDEBUG -g -O0
else
    CFLAGS += -DNDEBUG -O2
    CXXFLAGS += -DNDEBUG -O2
endif

CPPSOURCES = $(wildcard *.cpp)
CSOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.cpp, %.o, $(CPPSOURCES)) $(patsubst %.c, %.o, $(CSOURCES))

PARSE_OBJECTS = parser/parser.o parser/lexer.o
BCPARSE_OBJECTS = bcparser/parser.o bcparser/lexer.o

.PHONY: parser bcparser clean extraclean all

default: vm bccompiler interpreter pprinter

BIOBJECTS = $(patsubst %.cpp, %.o, $(wildcard interpreter/*.cpp))
interpreter: parser $(PARSE_OBJECTS) $(OBJECTS) $(BIOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(BIOBJECTS) -o bin/$@

COOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/*.cpp))
bccompiler: parser $(PARSE_OBJECTS) $(OBJECTS) $(COOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(COOBJECTS) -o bin/$@

PPOBJECTS = $(patsubst %.cpp, %.o, $(wildcard pprinter/*.cpp))
pprinter: parser $(PARSE_OBJECTS) $(OBJECTS) $(PPOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(PPOBJECTS) -o bin/$@

ASOBJECTS = $(patsubst %.cpp, %.o, $(wildcard astprinter/*.cpp))
PPASOBJECTS = pprinter/PrettyPrinter.o
astprinter: parser $(PARSE_OBJECTS) $(OBJECTS) pprinter/PrettyPrinter.o $(ASOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(OBJECTS) $(PPASOBJECTS) $(ASOBJECTS) -o bin/$@

VMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard vm/*.cpp))
BCCOOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/Compiler*.cpp))
GCOBJECTS = $(patsubst %.cpp, %.o, $(wildcard gc/*.cpp))
vm: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(VMOBJECTS) $(GCOBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(VMOBJECTS) $(GCOBJECTS) -o bin/$@

IROBJECTS = $(patsubst %.cpp, %.o, $(wildcard ir/*.cpp))
ir: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(IROBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(BCCOOBJECTS) $(IROBJECTS) -o bin/$@

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

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f bin/* *.o */*.o bytecode/*.o

extraclean: clean
	rm -f parser/*.c parser/*.cpp parser/*.h bcparser/*.c bcparser/*.cpp bcparser/*.h
