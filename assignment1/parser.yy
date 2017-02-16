%code requires{

#include <iostream>
#include <string>
#define YY_DECL int yylex (YYSTYPE* yylval, YYLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER
#include "lexer.h"
#endif

using namespace std;

//The macro below is used by bison for error reporting
//it comes from stacck overflow
//http://stackoverflow.com/questions/656703/how-does-flex-support-bison-location-exactly
#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }


#include "AST.h"
//If you need additional header files, put them here.


}


%define api.pure full
%parse-param {yyscan_t yyscanner} {Statement*& out}
%lex-param {yyscan_t yyscanner}
%locations
%define parse.error verbose

%code provides{
YY_DECL;
int yyerror(YYLTYPE * yylloc, yyscan_t yyscanner, Statement*& out, const char* message);
}



//The union directive defines a union type that will be used to store
//the return values of all the parse rules. We have initialized for you
//with an intconst field that you can use to store an integer, and a
//stmt field with a pointer to a statement. Note that one limitation
//is that you can only use primitive types and pointers in the union.
%union {
    bool boolconst;
    string strconst;
	int intconst;
    Block* block;
    Program* program;
    Statement* statement;
	Assignment* assignment;
}

//Below is where you define your tokens and their types.
//for example, we have defined for you a T_int token, with type intconst
//the type is the name of a field from the union above

%token<intconst> T_int
%token<strconst> T_str
%token<boolconst> T_bool
%token T_none

%token<strconst> T_name

%token T_if
%token T_else
%token T_while
%token T_global
%token T_return
%token T_fun

%token<strconst> T_comp_op
%token<strconst> T_arith_op
%token<strconst> T_prod_op


//Use the %type directive to specify the types of AST nodes produced by each production.
//For example, you will have a program non-terimnal in your grammar, and it will
//return a Statement*. As with tokens, the name of the type comes
//from the union defined earlier.

%type<program> Program
%type<block> Block
%type<statement> Statement
%type<assignment> Assignment

%start Program

//You must also define any associativity directives that are necessary
//to resolve ambiguities and properly parse the code.

%%

// Program

Program: Program Statement {
            $$ = $1;
            $1->block->Append($2);
        }
    | %empty { $$ = new Program(); }
    ;

// Block

Block: '{' StatementList '}';

StatementList: StatementList Statement {
                    $$ = $1;
                    $1->Append($2);
                }
            | %empty { $$ = new Block(); }
            ;

// Statement

Statement: Assignment | CallStatement | Global | IfStatement | WhileLoop | Return;

// Assignment

Assignment: LHS '=' Expression ';' { $$ = new Assignment($1, $2); };

// CallStatement

CallStatement: Call ';';

Call: LHS '(' ArgList ')' { $$ = new CallStatement($1, $2); };

ArgList: ArgList ',' Expression {
                $$ = $1;
                $1.push_back($2);
            }
        | %empty { $$ = vector<Expression>(); }
        ;

// Global

Global: T_global T_name ';' { $$ = new Global($2); };

// IfStatement

IfStatement: T_if '(' Expression ')' Block ElseBlock { $$ = new IfStatement($2, $3, $4); };

ElseBlock: T_else Block { $$ = $2 }
        | %empty { $$ = new Block(); }
        ;

// WhileLoop

WhileLoop: T_while '(' Expression ')' Block { $$ = new WhileLoop($2, $3); };

// Return

Return: T_return Expression ';' { $$ = new Return($2); };

// Expression

// TODO

// Units

LHS: LHS '.' T_name { $$ = new FieldDereference($1, $2); }
    | LHS '[' Expression ']' { $$ = new IndexExpression($1, $2); }
    | T_name { $$ = new LHS($1); }
    ;


%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Statement*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg;
  return 0;
}

