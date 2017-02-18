
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
comment (\/\/.*\n)

int_const [0-9][0-9]*
str_const \"((\\\")|[^"])*\"
bool_const true|false
none_const None

name [_[:alpha:]][_[:alnum:]]*

if_keyword if
else_keyword else
while_keyword while
global_keyword global
return_keyword return
fun_keyword fun

punctuation "="|"("|")"|"{"|"}"|";"|","|":"|"."

%{
// Initial declarations
// In this section of the file, you can define named regular expressions.
// like int_const and whitespace above
%}

%%

%{
  // Keywords
%}

{if_keyword} {
  return T_if;
}

{else_keyword} {
  return T_else;
}

{while_keyword} {
  return T_while;
}

{global_keyword} {
  return T_global;
}

{return_keyword} {
  return T_return;
}

{fun_keyword} {
  return T_fun;
}

%{
  // Constants
%}

{int_const} {
  yylval->intconst = atoi(yytext);
  return T_int;
}

{str_const} {
  yylval->strconst = new string(yytext);
  return T_str;
}

{bool_const} {
  yylval->boolconst = (string(yytext) == "true");
  return T_bool;
}

{none_const} {
  return T_none;
}

%{
  // Identifiers
%}

{name} {
  yylval->strconst = new string(yytext);
  return T_name;
}

%{
  // Operators and punctuation
%}

"<=" {
  return T_lte;
}

">=" {
  return T_gte;
}

"<" {
  return T_lt;
}

">" {
  return T_gt;
}

"==" {
  return T_eq;
}

"-" {
  return T_minus;
}

"+" {
  return T_plus;
}

"*" {
  return T_mul;
}

"/" {
  return T_div;
}

"&" {
  return T_and;
}

"|" {
  return T_or;
}

{punctuation} {
  return yytext[0];
}

%{
  // Skips
%}

{whitespace}   { /* skip */ }

{comment}      { /* skip */ }



%%

