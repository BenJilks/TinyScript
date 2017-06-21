#include "TinyScript.h"
#include <stdlib.h>

/* Parse an assign statement */
void ParseAssign()
{
	Token name = look;
	Match("Name", TOKEN_IDENTIFIER);
	Match("=", TOKEN_ASSIGN);

	ParseExpression();
	ParseStore(name.data);
	Match(";", TOKEN_LINE_END);
}

/* Parse a return statement */
void ParseReturn()
{
	Match("return", TOKEN_RETURN);
	ParseExpression();
	WriteLine("ireturn");
	Match(";", TOKEN_LINE_END);
}

/* Parse an if statement */
void ParseIf()
{
	Match("if", TOKEN_IF);
	ParseExpression();
	
	char* label = GenerateLabel();
	WriteLineVar("jumpifnot %s", label);
	ParseBlock();
	
	Emit(label);
	free(label);
}

/* Parse a single statement */
void ParseStatement()
{
	switch(look.id)
	{
		case TOKEN_IDENTIFIER: ParseAssign(); break;
		case TOKEN_RETURN: ParseReturn(); break;
		case TOKEN_IF: ParseIf(); break;
		default:
		{
			char msg[80];
			sprintf(msg, "Unexpected token '%s'", look.data);
			Abort(msg);
		}
	}
}

/* Parse a block of code */
void ParseBlock()
{
	/* Parse single line statement */
	if (look.id != TOKEN_OPEN_BLOCK)
	{
		ParseStatement();
		return;
	}
	
	/* Parse multi line statement */
	Match("{", TOKEN_OPEN_BLOCK);
	while(look.id != TOKEN_CLOSE_BLOCK)
		ParseStatement();
	Match("}", TOKEN_CLOSE_BLOCK);
}
