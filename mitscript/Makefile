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
CXXFLAGS = -std=c++1y
LDFLAGS =

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG -g -O0
    CXXFLAGS += -DDEBUG -g -O0
    LDFLAGS += -DDEBUG -g -O0
else
    CFLAGS += -DNDEBUG -O2
    CXXFLAGS += -DNDEBUG -O2
    LDFLAGS += -DNDEBUG -O2
endif

CPPSOURCES = $(wildcard *.cpp)
CSOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.cpp, %.o, $(CPPSOURCES)) $(patsubst %.c, %.o, $(CSOURCES))

PARSE_OBJECTS = parser/parser.o parser/lexer.o
BCPARSE_OBJECTS = bcparser/parser.o bcparser/lexer.o

.PHONY: parser bcparser clean extraclean

BIOBJECTS = $(patsubst %.cpp, %.o, $(wildcard interpreter/*.cpp))
interpreter: parser $(PARSE_OBJECTS) $(OBJECTS) $(BIOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(PARSE_OBJECTS) $(OBJECTS) $(BIOBJECTS)

COOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/*.cpp))
bccompiler: parser $(PARSE_OBJECTS) $(OBJECTS) $(COOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(PARSE_OBJECTS) $(OBJECTS) $(COOBJECTS)

PPOBJECTS = $(patsubst %.cpp, %.o, $(wildcard pprinter/*.cpp))
pprinter: parser $(PARSE_OBJECTS) $(OBJECTS) $(PPOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(PARSE_OBJECTS) $(OBJECTS) $(PPOBJECTS)

ASOBJECTS = $(patsubst %.cpp, %.o, $(wildcard astprinter/*.cpp))
PPASOBJECTS = pprinter/PrettyPrinter.o
astprinter: parser $(PARSE_OBJECTS) $(OBJECTS) pprinter/PrettyPrinter.o $(ASOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(PARSE_OBJECTS) $(OBJECTS) $(PPASOBJECTS) $(ASOBJECTS)

VMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard vm/*.cpp))
COVMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/Compiler*.cpp))
vm: parser bcparser $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(COVMOBJECTS) $(VMOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(PARSE_OBJECTS) $(BCPARSE_OBJECTS) $(OBJECTS) $(COVMOBJECTS) $(VMOBJECTS)

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
