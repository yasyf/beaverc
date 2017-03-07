CC = g++
CXX = g++
LEX = flex
YACC = bison

CFLAGS = -std=c++11 -g
CXXFLAGS = -std=c++11 -g
LDFLAGS =

interpret: lexer.c parser.cpp interpret.o parser.o lexer.o AST.o FunctionScanner.o Interpreter.o
	$(CXX) $(LDFLAGS) -o $@ interpret.o parser.o lexer.o AST.o FunctionScanner.o Interpreter.o

print: lexer.c parser.cpp print.o parser.o lexer.o AST.o PrettyPrinter.o
	$(CXX) $(LDFLAGS) -o $@ print.o parser.o lexer.o AST.o PrettyPrinter.o

astprint: lexer.c parser.cpp astprint.o parser.o lexer.o AST.o PrettyPrinter.o ASTPrinter.o
	$(CXX) $(LDFLAGS) -o $@ astprint.o parser.o lexer.o AST.o PrettyPrinter.o ASTPrinter.o

lexer.c: lexer.lex
	$(LEX) --debug --header-file=lexer.h -o lexer.c lexer.lex

parser.cpp: parser.yy
	$(YACC) --debug --defines=parser.h -o parser.cpp parser.yy

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f astprint print interpret *.o

extraclean: clean
	rm -f lexer.c lexer.h parser.cpp parser.h
