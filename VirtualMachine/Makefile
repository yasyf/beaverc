all: parser.cpp lexer.cpp bc-parser.cpp
	g++ -g -std=c++1y bc-parser.cpp parser.cpp lexer.cpp -o bc-parser
	g++ -I../ParsingAndInterpreting -g -std=c++1y compiler.cpp ../ParsingAndInterpreting/parser.cpp ../ParsingAndInterpreting/lexer.cpp -o compiler


parser.cpp: parser.yy
	bison -Dapi.prefix=bc --output=parser.cpp --defines=parser.h -v parser.yy

lexer.cpp: lexer.lex
	flex -P bc --outfile=lexer.cpp --header-file=lexer.h lexer.lex

clean: 
	rm -f lexer.cpp lexer.h parser.cpp parser.h parser.output *.exe
