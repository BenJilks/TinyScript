%option noyywrap
%{
#include <stdlib.h>
#include "parser.h"
%}

digit 	[0-9]
letter	([a-z]|[A-Z])
num 	{digit}+
ident	{letter}({letter}|{digit}|_)*

%%

[ \t]*	{}
[\n]	{ yylineno++; }

"func"	{ return FUNC; }
{num}	{ yylval.int_val = atoi(yytext); return NUM; }
{ident}	{ yylval.str_val = new string(yytext); return IDENT; }
.	{ return yytext[0]; }

%%

