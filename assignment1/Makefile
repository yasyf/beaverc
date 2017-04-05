CC = g++
CXX = g++
LEX = flex
YACC = bison

CFLAGS = -std=c++1y -g
CXXFLAGS = -std=c++1y -g
LDFLAGS =

interpreter: lexer.c parser.cpp interpret.o parser.o lexer.o AST.o FunctionScanner.o Interpreter.o
	$(CXX) $(LDFLAGS) -o $@ interpret.o parser.o lexer.o AST.o FunctionScanner.o Interpreter.o

printer: lexer.c parser.cpp print.o parser.o lexer.o AST.o PrettyPrinter.o
	$(CXX) $(LDFLAGS) -o $@ print.o parser.o lexer.o AST.o PrettyPrinter.o

astprinter: lexer.c parser.cpp astprint.o parser.o lexer.o AST.o PrettyPrinter.o ASTPrinter.o
	$(CXX) $(LDFLAGS) -o $@ astprint.o parser.o lexer.o AST.o PrettyPrinter.o ASTPrinter.o

bcprinter: lexer.c parser.cpp bytecode/bcprint.o parser.o lexer.o AST.o bytecode/TranspilerFunctionScanner.o bytecode/Transpiler.o
	$(CXX) $(LDFLAGS) -o $@ bytecode/bcprint.o parser.o lexer.o AST.o bytecode/TranspilerFunctionScanner.o bytecode/Transpiler.o

lexer.c: lexer.lex
	$(LEX) --debug --header-file=lexer.h -o lexer.c lexer.lex

parser.cpp: parser.yy
	$(YACC) --debug --defines=parser.h -o parser.cpp parser.yy

%.o: %.cpp %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f bcprinter astprinter printer interpreter *.o bytecode/*.o

extraclean: clean
	rm -f lexer.c lexer.h parser.cpp parser.h
