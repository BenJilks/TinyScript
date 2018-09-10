%option noyywrap
%{
#include <stdlib.h>
#include "parser.h"

#define LIT_VAL(type_n, name, value) \
	yylval.lit_val = new Literal(); \
	yylval.lit_val->name = value; \
	yylval.lit_val->type = PrimType::type_n;
%}

digit 	[0-9]
letter	([a-z]|[A-Z])
int 	{digit}+
float	({digit}+\.{digit}*)|({digit}*\.{digit}+)
char	'.'
string	\".*\"
ident	{letter}({letter}|{digit}|_)*

%%

[ \t]*		{}
[\n]		{ yylineno++; }

"func"		{ return FUNC; }
"true"		{ LIT_VAL(Bool, b, true); return BOOL_LIT; }
"false"		{ LIT_VAL(Bool, b, false); return BOOL_LIT; }
{float}		{ LIT_VAL(Float, f, atof(yytext)); return FLOAT_LIT; }
{int}		{ LIT_VAL(Int, i, atoi(yytext)); return INT_LIT; }
{char}		{ LIT_VAL(Char, c, yytext[1]); return CHAR_LIT; }
{string}	{ LIT_VAL(String, s, new string(yytext + 1, strlen(yytext) - 2)); return STRING_LIT; }
{ident}		{ yylval.str_val = new string(yytext); return IDENT; }
.		{ return yytext[0]; }

%%

