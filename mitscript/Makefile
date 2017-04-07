CC = g++
CXX = g++
LEX = flex
YACC = bison

CFLAGS = -std=c++1y -g
CXXFLAGS = -std=c++1y -g
LDFLAGS =

CPPSOURCES = $(wildcard *.cpp) $(wildcard parser/*.cpp)
CSOURCES = $(wildcard *.c) $(wildcard parser/*.c)
OBJECTS = $(patsubst %.cpp, %.o, $(CPPSOURCES)) $(patsubst %.c, %.o, $(CSOURCES))

.PHONY: parser clean extraclean

BIOBJECTS = $(patsubst %.cpp, %.o, $(wildcard interpreter/*.cpp))
interpreter: parser $(OBJECTS) $(BIOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(OBJECTS) $(BIOBJECTS)

COOBJECTS = $(patsubst %.cpp, %.o, $(wildcard bccompiler/*.cpp))
bccompiler: parser $(OBJECTS) $(COOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(OBJECTS) $(COOBJECTS)

PPOBJECTS = $(patsubst %.cpp, %.o, $(wildcard pprinter/*.cpp))
pprinter: parser $(OBJECTS) $(PPOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(OBJECTS) $(PPOBJECTS)

ASOBJECTS = $(patsubst %.cpp, %.o, $(wildcard astprinter/*.cpp))
astprinter: parser $(OBJECTS) pprinter/PrettyPrinter.o $(ASOBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ $(OBJECTS) pprinter/PrettyPrinter.o $(ASOBJECTS)

parser: parser/lexer.c parser/parser.cpp

parser/lexer.c:
	$(LEX) --debug --header-file=parser/lexer.h -o parser/lexer.c parser/lexer.lex

parser/parser.cpp:
	$(YACC) --debug --defines=parser/parser.h -o parser/parser.cpp parser/parser.yy

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f bin/* *.o */*.o bytecode/*.o

extraclean: clean
	rm -f parser/lexer.c parser/lexer.h parser/parser.cpp parser/parser.h
