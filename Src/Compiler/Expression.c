#include "TinyScript.h"
#include <string.h>
#include <stdlib.h>

/* 
	<bool-expression> ::= <bool-factor> (<andop> <bool-factor>)*
	<bool-factor> ::= <identifier> | <bool> | <relation>
	<relation> ::= <expression> (<relop> <expression>)*

	<value-expression> ::= <term> (<addop> <term>)*
	<term> ::= <factor> (<mulop> <factor>)*
	<factor> ::= <identifier> | <number> | (<expression>)
*/

/* TYPE_CHECK_ID */
#define TCI(ltype, rtype) (rtype) * DT_SIZE + (ltype)

/* Write the type correct operation code */
int TypeChecked(char* code_gen, int ltype, int rtype)
{
	switch(TCI(ltype, rtype))
	{
		case TCI(DT_FLOAT, DT_FLOAT): // float and float
			WriteLineVar("%sf", code_gen); return DT_FLOAT;
		
		case TCI(DT_INT, DT_FLOAT): case TCI(DT_CHAR, DT_FLOAT): // int/char and float (int last on stack)
			WriteLine("itofright"); WriteLineVar("%sf", code_gen); return DT_FLOAT;
		
		case TCI(DT_FLOAT, DT_INT): case TCI(DT_FLOAT, DT_CHAR): // float int int/char (float last on stack)
			WriteLine("itofleft"); WriteLineVar("%sf", code_gen); return DT_FLOAT;
	}
	WriteLine(code_gen);
	return ltype;
}

/* Parse a factor */
int ParseFactor()
{
	/* If the factor is another expression */
	if (look.id == TOKEN_OPEN_ARG)
	{
		Match("(", TOKEN_OPEN_ARG);
		ParseExpression();
		Match(")", TOKEN_CLOSE_ARG);
		return DT_INT;
	}

	/* If the factor is a variable of function */
	if (look.id == TOKEN_IDENTIFIER)
	{
		int type = ParseLoad();
		if (type == DT_VOID)
			Abort("Cannot assign to type 'void'");
		return type;
	}

	/* If the value is a float type */
	if (look.id == TOKEN_FLOAT)
	{
		float f = atof(look.data);
		int i = *(int*)&f;
		WriteLineVar("push %i", i);
		Match("Value", TOKEN_FLOAT);
		return DT_FLOAT;
	}
	
	/* Otherwise, push the value */
	WriteLineVar("push %s", look.data);
	Match("Value", TOKEN_NUMBER);
	return DT_INT;
}

/* Parse a mulop */
int ParseMulop(int token, char* name, char* code_gen, int ltype)
{
	Match(name, token);
	int rtype = ParseFactor();
	return TypeChecked(code_gen, ltype, rtype);
}

/* Parse an expression */
int ParseTerm()
{
	int ltype = ParseFactor();
	while (look.id == TOKEN_MUL || look.id == TOKEN_DIV)
	{
		switch(look.id)
		{
			case TOKEN_MUL: return ParseMulop(TOKEN_MUL, "*", "mul", ltype);
			case TOKEN_DIV: return ParseMulop(TOKEN_DIV, "/", "div", ltype);
		}
	}
	return ltype;
}

/* Parse an addop */
int ParseAddop(int token, char* name, char* code_gen, int ltype)
{
	Match(name, token);
	int rtype = ParseTerm();
	return TypeChecked(code_gen, ltype, rtype);
}

int ParseValueExpression()
{
	int ltype = ParseTerm();
	while (look.id == TOKEN_ADD || look.id == TOKEN_SUB)
	{
		switch(look.id)
		{
			case TOKEN_ADD: return ParseAddop(TOKEN_ADD, "+", "add", ltype);
			case TOKEN_SUB: return ParseAddop(TOKEN_SUB, "-", "sub", ltype);
		}
	}
	return ltype;
}

/* Checks if the look token contains a relop */
int IsRelop()
{
	return look.id == TOKEN_MORE || look.id == TOKEN_LESS ||
		look.id == TOKEN_MORE_EQUAL || look.id == TOKEN_LESS_EQUAL || 
		look.id == TOKEN_EQUAL || look.id == TOKEN_NOT_EQUAL;
}

/* Parse a generic relop operation */
int ParseRelop(int token, char* name, char* code_gen, int ltype)
{
	Match(name, token);
	int rtype = ParseValueExpression();
	return TypeChecked(code_gen, ltype, rtype);
}

/* Parse a relation */
int ParseRelation()
{
	int ltype = ParseValueExpression();
	while (IsRelop())
	{
		switch(look.id)
		{
			case TOKEN_MORE: return ParseRelop(TOKEN_MORE, ">", "more", ltype);
			case TOKEN_LESS: return ParseRelop(TOKEN_LESS, "<", "less", ltype);
			case TOKEN_MORE_EQUAL: return ParseRelop(TOKEN_MORE_EQUAL, ">=", "emore", ltype);
			case TOKEN_LESS_EQUAL: return ParseRelop(TOKEN_LESS_EQUAL, "<=", "eless", ltype);
			case TOKEN_EQUAL: return ParseRelop(TOKEN_EQUAL, "==", "eqaul", ltype);
			case TOKEN_NOT_EQUAL: return ParseRelop(TOKEN_NOT_EQUAL, "!=", "neql", ltype);
		}
	}
	return ltype;
}

/* Parse a boolean factor */
int ParseBoolFactor()
{
	if (look.id == TOKEN_BOOL)
	{
		WriteLineVar("push %s", strcmp(look.data, "true") ? "0" : "1");
		Match("Bool", TOKEN_BOOL);
		return DT_INT;
	}

	return ParseRelation();
}

/* Parse an andop */
int ParseAndop(int token, char* name, char* code_gen, int ltype)
{
	Match(name, token);
	int rtype = ParseBoolFactor();
	return TypeChecked(code_gen, ltype, rtype);
}

/* Parse a boolean expression */
int ParseBoolExpression()
{
	int ltype = ParseBoolFactor();
	while (look.id == TOKEN_AND)
	{
		switch(look.id)
		{
			case TOKEN_AND: return ParseAndop(TOKEN_AND, "&&", "and", ltype);
		}
	}
	return ltype;
}

/* Map the general expression parser to the boolean expression parser */
int ParseExpression() { return ParseBoolExpression(); }
