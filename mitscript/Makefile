CC = g++
CXX = g++
LEX = flex
YACC = bison

CFLAGS = -std=c++1y -g
CXXFLAGS = -std=c++1y -g
LDFLAGS =

CPPSOURCES=$(wildcard *.cpp) $(wildcard */*.cpp)
CSOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.cpp, %.o, $(CPPSOURCES)) $(patsubst %.c, %.o, $(CSOURCES))

interpreter: lexer.c parser.cpp $(OBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ interpret.o parser.o lexer.o AST.o FunctionScanner.o Interpreter.o

printer: lexer.c parser.cpp $(OBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ print.o parser.o lexer.o AST.o PrettyPrinter.o

astprinter: lexer.c parser.cpp $(OBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ astprint.o parser.o lexer.o AST.o PrettyPrinter.o ASTPrinter.o

bcprinter: lexer.c parser.cpp $(OBJECTS)
	$(CXX) $(LDFLAGS) -o bin/$@ bytecode/bcprint.o parser.o lexer.o AST.o bytecode/CompilerFunctionScanner.o bytecode/Compiler.o

lexer.c: lexer.lex
	$(LEX) --debug --header-file=lexer.h -o lexer.c lexer.lex

parser.cpp: parser.yy
	$(YACC) --debug --defines=parser.h -o parser.cpp parser.yy

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f bin/* *.o bytecode/*.o

extraclean: clean
	rm -f lexer.c lexer.h parser.cpp parser.h
