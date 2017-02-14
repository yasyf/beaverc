
%{

#include <string>
#include "parser.h"
using std::string;
// You can put additional header files here.

%}

%option reentrant
%option noyywrap
%option never-interactive

whitespace   ([ \t\n]*)
comment \/\/.*\n

int_const [0-9][0-9]*
str_const "((\\") | [^"])*"
bool_const true | false
none_const None

name [_[:alpha:]][_[:alnum:]]*

if_keyword if
else_keyword else
while_keyword while
global_keyword global
return_keyword return
fun_keyword fun

comparison_operator < | > | <= | >=| ==
arithmetic_operator + | -
product_operator \* | \/


%{
// Initial declarations
// In this section of the file, you can define named regular expressions.
// like int_const and whitespace above
%}

%%


{whitespace}   { /* skip */ }

{comment}      { /* skip */ }


{int_const}    {
      yylval->intconst = atoi(yytext);
      return T_int;
    }

{str_const}    {

      yylval->str_const = string(yytext);
      return T_str;
    }

{bool_const}    {
      yylval->str_const = (string(yytext) == "true");
      return T_bool;
    }

{none_const}    {
      return T_none;
    }

{name}    {
			yylval->name = string(yytext);
      return T_name;
		}

%{
// The rest of your lexical rules go here.
// rules have the form
// pattern action
// we have defined a few rules for you above, but you need
// to provide additional lexical rules for string constants,
// operators, keywords and identifiers.
%}

%%

