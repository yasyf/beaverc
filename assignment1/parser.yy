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
%parse-param {yyscan_t yyscanner} {Program*& out}
%lex-param {yyscan_t yyscanner}
%locations
%define parse.error verbose

%code provides{
YY_DECL;
int yyerror(YYLTYPE * yylloc, yyscan_t yyscanner, Program*& out, const char* message);
}



//The union directive defines a union type that will be used to store
//the return values of all the parse rules. We have initialized for you
//with an intconst field that you can use to store an integer, and a
//stmt field with a pointer to a statement. Note that one limitation
//is that you can only use primitive types and pointers in the union.
%union {
    bool boolconst;
    string* strconst;
    int intconst;
    Block* block;
    Program* program;
    Statement* statement;
    Assignment* assignment;
    CallStatement* callstmt;
    Global* global;
    IfStatement* ifstmt;
    WhileLoop* whileloop;
    Return* retstmt;
    Expression* expr;
    Function* func;
    Record* record;
    Unit* unit;
    LHS* lhs;
    Call* call;
    vector<Expression *>* exprvec;
    vector<Name *>* namevec;
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

%left T_or "|"
%left T_and "&"
%left T_not "!"
%left T_lt "<"
%left T_lte "<="
%left T_gt ">"
%left T_gte ">="
%left T_eq "=="
%left T_plus "+"
%left T_minus "-"
%left T_mul "*"
%left T_div "/"

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
%type<whileloop> WhileLoop
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
%type<expr> PositiveUnit
%type<unit> Constant
%type<lhs> LHS
%type<call> Call
%type<exprvec> ArgList
%type<exprvec> FullArgList
%type<exprvec> EmptyArgList
%type<namevec> NameList

%start Program

%%

// Program

Program: Program Statement {
            $$ = $1;
            $1->block->Append($2);
        }
    | %empty { $$ = new Program(); out = $$; }
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

Global: T_global T_name ';' { $$ = new Global(*$2); };

// IfStatement

IfStatement: T_if '(' Expression ')' Block ElseBlock { $$ = new IfStatement($3, $5, $6); };

ElseBlock: T_else Block { $$ = $2; }
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

Function: T_fun '(' NameList ')' Block { $$ = new Function(*$3, $5); };

// Boolean

Boolean: Boolean T_or Conjunction { $$ = new BinaryOp<OR>($1, $3); }
        | Conjunction
        ;

Conjunction: Conjunction T_and BoolUnit { $$ = new BinaryOp<AND>($1, $3); }
        | BoolUnit
        ;

BoolUnit: T_not Predicate { $$ = new UnaryOp<NOT>($2); }
        | Predicate
        ;

Predicate: Predicate T_lt Arithmetic { $$ = new BinaryOp<LT>($1, $3); }
        | Predicate T_lte Arithmetic { $$ = new BinaryOp<LTE>($1, $3); }
        | Predicate T_gt Arithmetic { $$ = new BinaryOp<GT>($1, $3); }
        | Predicate T_gte Arithmetic { $$ = new BinaryOp<GTE>($1, $3); }
        | Predicate T_eq Arithmetic { $$ = new BinaryOp<EQ>($1, $3); }
        | Arithmetic
        ;

Arithmetic: Arithmetic T_plus Product { $$ = new BinaryOp<PLUS>($1, $3); }
        | Arithmetic T_minus Product { $$ = new BinaryOp<MINUS>($1, $3); }
        | Product
        ;

Product: Product T_mul Unit { $$ = new BinaryOp<MUL>($1, $3); }
    | Product T_div Unit { $$ = new BinaryOp<DIV>($1, $3); }
    | Unit
    ;

// Record

Record: '{' RecordContents '}' { $$ = $2; };

RecordContents: RecordContents T_name ':' Expression ';' {
                    $$ = $1;
                    $1->Add(*$2, $4);
                }
            | %empty { $$ = new Record(); }
            ;

// Units

Unit: T_minus PositiveUnit { $$ = new UnaryOp<NEG>($2); }
    | PositiveUnit       { $$ = $1; }
    ;

PositiveUnit: LHS             { $$ = $1; }
            | Constant        { $$ = $1; }
            | Call            { $$ = $1; }
            | '(' Boolean ')' { $$ = $2; }
            ;

Constant: T_int { $$ = new Constant<int>($1); }
        | T_str { $$ = new Constant<string>(*$1); }
        | T_bool { $$ = new Constant<bool>($1); }
        ;

LHS: LHS '.' T_name { $$ = new FieldDereference($1, new Name(*$3)); }
    | LHS '[' Expression ']' { $$ = new IndexExpression($1, $3); }
    | T_name { $$ = new Name(*$1); }
    ;

Call: LHS '(' ArgList ')' { $$ = new Call($1, *$3); };

ArgList: EmptyArgList | FullArgList;

EmptyArgList: %empty { $$ = new vector<Expression *>(); };

FullArgList: FullArgList ',' Expression {
                $$ = $1;
                $1->push_back($3);
            }
        | Expression {
            $$ = new vector<Expression *>{$1};
        }
        ;

NameList: NameList T_name {
                $$ = $1;
                $1->push_back(new Name(*$2));
            }
        | %empty { $$ = new vector<Name *>(); }
        ;

%%

// Error reporting function. You should not have to modify this.
int yyerror(YYLTYPE * yylloc, void* p, Program*& out, const char*  msg){

  cout<<"Error in line "<<yylloc->last_line<<", col "<<yylloc->last_column<<": "<<msg<<endl;
  return 0;
}

