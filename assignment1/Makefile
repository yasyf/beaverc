CC = g++
CXX = g++
LEX = flex
YACC = bison

CXXFLAGS = -std=c++11
LDFLAGS =

HEADERS = lexer.h parser.h AST.h

main: lexer.c parser.cpp main.o parser.o lexer.o AST.o
	$(CXX) $(LDFLAGS) -o $@ main.o parser.o lexer.o AST.o

lexer.c: lexer.lex
	$(LEX) --debug --header-file=lexer.h -o lexer.c lexer.lex

parser.cpp: parser.yy
	$(YACC) --debug --defines=parser.h -o parser.cpp parser.yy

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f main *.o

extraclean: clean
	rm -f lexer.c lexer.h parser.cpp parser.h
