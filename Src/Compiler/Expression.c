#include "TinyScript.h"
#include <string.h>

/* 
	<bool-expression> ::= <bool-factor> (<andop> <bool-factor>)*
	<bool-factor> ::= <identifier> | <bool> | <relation>
	<relation> ::= <expression> (<relop> <expression>)*

	<value-expression> ::= <term> (<addop> <term>)*
	<term> ::= <factor> (<mulop> <factor>)*
	<factor> ::= <identifier> | <number> | (<expression>)
*/

/* Parse a factor */
void ParseFactor()
{
	/* If the factor is another expression */
	if (look.id == TOKEN_OPEN_ARG)
	{
		Match("(", TOKEN_OPEN_ARG);
		ParseExpression();
		Match(")", TOKEN_CLOSE_ARG);
		return;
	}

	/* If the factor is a variable of function */
	if (look.id == TOKEN_IDENTIFIER)
	{
		ParseLoad();
		return;
	}

	/* Otherwise, push the value */
	WriteLineVar("push %s", look.data);
	Match("Value", TOKEN_NUMBER);
}

/* Parse a mulop */
void ParseMulop(int token, char* name, char* code_gen)
{
	Match(name, token);
	ParseFactor();
	WriteLine(code_gen);
}

/* Parse an expression */
void ParseTerm()
{
	ParseFactor();
	while (look.id == TOKEN_MUL || look.id == TOKEN_DIV)
	{
		switch(look.id)
		{
			case TOKEN_MUL: ParseMulop(TOKEN_MUL, "*", "mul"); break;
			case TOKEN_DIV: ParseMulop(TOKEN_DIV, "/", "div"); break;
		}
	}
}

/* Parse an addop */
void ParseAddop(int token, char* name, char* code_gen)
{
	Match(name, token);
	ParseTerm();
	WriteLine(code_gen);
}

void ParseValueExpression()
{
	ParseTerm();
	while (look.id == TOKEN_ADD || look.id == TOKEN_SUB)
	{
		switch(look.id)
		{
			case TOKEN_ADD: ParseAddop(TOKEN_ADD, "+", "add"); break;
			case TOKEN_SUB: ParseAddop(TOKEN_SUB, "-", "sub"); break;
		}
	}
}

/* Checks if the look token contains a relop */
int IsRelop()
{
	return look.id == TOKEN_MORE || look.id == TOKEN_LESS ||
		look.id == TOKEN_MORE_EQUAL || look.id == TOKEN_LESS_EQUAL || 
		look.id == TOKEN_EQUAL || look.id == TOKEN_NOT_EQUAL;
}

/* Parse a generic relop operation */
void ParseRelop(int token, char* name, char* code_gen)
{
	Match(name, token);
	ParseValueExpression();
	WriteLine(code_gen);
}

/* Parse a relation */
void ParseRelation()
{
	ParseValueExpression();
	while (IsRelop())
	{
		switch(look.id)
		{
			case TOKEN_MORE: ParseRelop(TOKEN_MORE, ">", "more"); break;
			case TOKEN_LESS: ParseRelop(TOKEN_LESS, "<", "less"); break;
			case TOKEN_MORE_EQUAL: ParseRelop(TOKEN_MORE_EQUAL, ">=", "emore"); break;
			case TOKEN_LESS_EQUAL: ParseRelop(TOKEN_LESS_EQUAL, "<=", "eless"); break;
			case TOKEN_EQUAL: ParseRelop(TOKEN_EQUAL, "==", "eqaul"); break;
			case TOKEN_NOT_EQUAL: ParseRelop(TOKEN_NOT_EQUAL, "!=", "neql"); break;
		}
	}
}

/* Parse a boolean factor */
void ParseBoolFactor()
{
	if (look.id == TOKEN_BOOL)
	{
		WriteLineVar("push %s", strcmp(look.data, "true") ? "0" : "1");
		Match("Bool", TOKEN_BOOL);
		return;
	}

	ParseRelation();
}

/* Parse an andop */
void ParseAndop(int token, char* name, char* code_gen)
{
	Match(name, token);
	ParseBoolFactor();
	WriteLine(code_gen);
}

/* Parse a boolean expression */
void ParseBoolExpression()
{
	ParseBoolFactor();
	while (look.id == TOKEN_AND)
	{
		switch(look.id)
		{
			case TOKEN_AND: ParseAndop(TOKEN_AND, "&&", "and"); break;
		}
	}
}

/* Map the general expression parser to the boolean expression parser */
void ParseExpression() { ParseBoolExpression(); }
