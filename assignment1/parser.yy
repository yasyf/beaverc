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
    CallStatement* callstmt;
    Global* global;
    IfStatement* ifstmt;
    WhileLoop* while;
    Return* retstmt;
    Expression* expr;
    Function* func;
    Record* record;
    Unit* unit;
    Constant* constant;
    LHS* lhs;
    Call* call;
    vector<Expression>* arglist;
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

%left OR "|"
%left AND "&"
%left NOT "!"
%left LT "<"
%left LTE "<="
%left GT ">"
%left GTE ">="
%left EQ "=="
%left PLUS "+"
%left MINUS "-"
%left MUL "*"
%left DIV "/"

//Use the %type directive to specify the types of AST nodes produced by each production.
//For example, you will have a program non-terimnal in your grammar, and it will
//return a Statement*. As with tokens, the name of the type comes
//from the union defined earlier.

%type<program> Program
%type<block> Block
%type<block> StatementList
%type<statement> Statement
%type<assignment> Assignment
%type<callstmt> CallStatement
%type<global> Global
%type<ifstmt> IfStatement
%type<while> WhileLoop
%type<retstmt> Return
%type<block> ElseBlock
%type<expr> Expression
%type<func> Function
%type<expr> Boolean
%type<expr> Conjunction
%type<expr> BoolUnit
%type<expr> Predicate
%type<expr> Arithmetic
%type<expr> Product
%type<record> Record
%type<record> RecordContents
%type<expr> Unit
%type<unit> PositiveUnit
%type<constant> Constant
%type<lhs> LHS
%type<call> Call
%type<arglist> ArgList

%start Program

%%

// Program

Program: Program Statement {
            $$ = $1;
            $1->block->Append($2);
        }
    | %empty { $$ = new Program(); }
    ;

// Block

Block: '{' StatementList '}' { $$ = $2; };

StatementList: StatementList Statement {
                    $$ = $1;
                    $1->Append($2);
                }
            | %empty { $$ = new Block(); }
            ;

// Statement

Statement: Assignment   { $$ = $1; }
        | CallStatement { $$ = $1; }
        | Global        { $$ = $1; }
        | IfStatement   { $$ = $1; }
        | WhileLoop     { $$ = $1; }
        | Return        { $$ = $1; }
        ;

// Assignment

Assignment: LHS '=' Expression ';' { $$ = new Assignment($1, $3); };

// CallStatement

CallStatement: Call ';' { $$ = new CallStatement($1); };

// Global

Global: T_global T_name ';' { $$ = new Global($2); };

// IfStatement

IfStatement: T_if '(' Expression ')' Block ElseBlock { $$ = new IfStatement($3, $5, $6); };

ElseBlock: T_else Block { $$ = $2 }
        | %empty { $$ = new Block(); }
        ;

// WhileLoop

WhileLoop: T_while '(' Expression ')' Block { $$ = new WhileLoop($3, $5); };

// Return

Return: T_return Expression ';' { $$ = new Return($2); };

// Expression

Expression: Function { $$ = $1; }
        | Boolean { $$ = $1; }
        | Record  { $$ = $1; }
        ;

// Function

Function: T_fun '(' ArgList ')' Block { $$ = new Function($3, *$5); };

// Boolean

Boolean: Boolean OR Conjunction { $$ = new BinaryOp<OR>($1, $3); }
        | Conjunction
        ;

Conjunction: Conjunction AND BoolUnit { $$ = new BinaryOp<AND>($1, $3); }
        | BoolUnit
        ;

BoolUnit: NOT Predicate { $$ = new UnaryOp<NOT>($2); }
        | Predicate
        ;

Predicate: Predicate LT Arithmetic { $$ = new BinaryOp<LT>($1, $3); }
        | Predicate LTE Arithmetic { $$ = new BinaryOp<LTE>($1, $3); }
        | Predicate GT Arithmetic { $$ = new BinaryOp<GT>($1, $3); }
        | Predicate GTE Arithmetic { $$ = new BinaryOp<GTE>($1, $3); }
        | Predicate EQ Arithmetic { $$ = new BinaryOp<EQ>($1, $3); }
        | Arithmetic
        ;

Arithmetic: Arithmetic PLUS Product { $$ = BinaryOp<PLUS>($1, $3); }
        | Arithmetic MINUS Product { $$ = BinaryOp<MINUS>($1, $3); }
        | Product
        ;

Product: Product MUL Unit { $$ = new BinaryOp<MUL>($1, $3); }
    | Product DIV Unit { $$ = new BinaryOp<DIV>($1, $3); }
    | Unit
    ;

// Record

Record: '{' RecordContents '}' { $$ = $2; };

RecordContents: RecordContents T_name ':' Expression ';' {
                    $$ = $1;
                    $1.Add($2, $4);
                }
            | %empty { $$ = new Record(); }
            ;

// Units

Unit: MINUS PositiveUnit { $$ = new UnaryOp<MINUS>($2); }
    | PositiveUnit       { $$ = $1; }
    ;

PositiveUnit: LHS             { $$ = $1; }
            | Constant        { $$ = $1; }
            | Call            { $$ = $1; }
            | '(' Boolean ')' { $$ = $2; }
            ;

Constant: T_int { $$ = new IntConstant($1); }
        | T_str { $$ = new StringConstant($1); }
        | T_bool { $$ = new BoolConstant($1); }
        ;

LHS: LHS '.' T_name { $$ = new FieldDereference($1, $3); }
    | LHS '[' Expression ']' { $$ = new IndexExpression($1, $3); }
    | T_name { $$ = new LHS($1); }
    ;

Call: LHS '(' ArgList ')' { $$ = new Call($1, *$3); };

ArgList: ArgList ',' Expression {
                $$ = $1;
                $1->push_back($3);
            }
        | %empty { $$ = new vector<Expression>(); }
        ;

%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Statement*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg;
  return 0;
}

