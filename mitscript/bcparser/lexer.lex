
%{

#include <string>
#include "parser.h"
using std::string;
// You can put additional header files here.

%}

%option reentrant
%option noyywrap
%option never-interactive
%option prefix="bc"

int_const -?[0-9][0-9]*

whitespace   ([ \t\n]*)
%{
// Initial declarations
// In this section of the file, you can define named regular expressions.
// like int_const and whitespace above
//begin_student_code
%}
name	[a-zA-Z_][a-zA-Z0-9_]*


string_const ("\""[^\n\"]*"\"")

Operator     ([\%\/\<\>\;\!\?\*\-\+\,\.\:\[\]\(\)\{\}\=\|\&\^\$])


comment      ("//"[^\n]*)
%{
//end_student_code
%}

%%


{whitespace}   { /* skip */ }

{comment}      { /* skip */ }


{int_const}    {
		//Rule to identify an integer constant.
		//The return value indicates the type of token;
		//in this case T_int as defined in parser.yy.
		//The actual value of the constant is returned
		//in the intconst field of yylval (defined in the union
		//type in parser.yy).
			yylval->intconst = atoi(yytext);
			return TBC_T_int;
		}

%{
// The rest of your lexical rules go here.
// rules have the form
// pattern action
// we have defined a few rules for you above, but you need
// to provide additional lexical rules for string constants,
// operators, keywords and identifiers.
//begin_student_code
%}


{string_const}  {

			string*  tmp = new string(yytext);
			*tmp = tmp->substr(1, tmp->size() -2);
			yylval->strconst = tmp;
			return TBC_T_string;
		}



"None" 		{ return TBC_T_none; }
"true" 		{ return TBC_T_true; }
"false"		{ return TBC_T_false; }
"function"  { return TBC_T_function; }
"functions" { return TBC_T_functions; }
"constants" { return TBC_T_constants; }
"parameter_count" { return TBC_T_parameter_count; }
"local_vars" { return TBC_T_local_vars; }
"local_ref_vars" { return TBC_T_local_ref_vars; }
"names" { return TBC_T_names; }
"free_vars" { return TBC_T_free_vars; }
"instructions" { return TBC_T_instructions; }

"load_const" { return TBC_T_load_const; }
"load_func" { return TBC_T_load_func; }
"load_local" { return TBC_T_load_local; }
"store_local" { return TBC_T_store_local; }
"load_global" { return TBC_T_load_global; }
"store_global" { return TBC_T_store_global; }
"push_ref"     { return TBC_T_push_ref; }
"load_ref"     { return TBC_T_load_ref; }
"store_ref"    { return TBC_T_store_ref; }
"alloc_record" { return TBC_T_alloc_record; }
"field_load"   { return TBC_T_field_load; }
"field_store"  { return TBC_T_field_store; }
"index_load"   { return TBC_T_index_load; }
"index_store"  { return TBC_T_index_store; }
"alloc_closure" { return TBC_T_alloc_closure; }
"call"  { return TBC_T_call; }
"return" { return TBC_T_return; }
"add" { return TBC_T_add; }
"sub" { return TBC_T_sub; }
"mul" { return TBC_T_mul; }
"div" { return TBC_T_div; }
"neg" { return TBC_T_neg; }
"gt" { return TBC_T_gt; }
"geq" { return TBC_T_geq; }
"eq" { return TBC_T_eq; }
"and" { return TBC_T_and; }
"or" { return TBC_T_or; }
"not" { return TBC_T_not; }
"goto" { return TBC_T_goto; }
"if"  { return TBC_T_if; }
"dup" { return TBC_T_dup; }
"swap" { return TBC_T_swap; }
"pop" {return TBC_T_pop; }

{Operator} {  return yytext[0]; }

{name} 		{
			yylval->strconst = new std::string(yytext);
			return TBC_T_ident;
		}

%{
//end_student_code
%}

%%

