#include "TinyScript.h"
#include <stdlib.h>

/* Parse a local variable delare statement */
void ParseDeclare()
{
	Match("declare", TOKEN_DECLARE);
	Token type = look;
	Match("Type", TOKEN_IDENTIFIER);
	
	while (look.id != TOKEN_LINE_END)
	{
		Token name = look;
		Match("Name", TOKEN_IDENTIFIER);
		CreateLocalVariable(name.data, type.data);
		
		if (look.id == TOKEN_LIST)
			Match(",", TOKEN_LIST);
	}
	Match(";", TOKEN_LINE_END);
}

/* Parse a function call */
void ParseFunctionCall(Token name)
{
	/* Get the symbol */
	Symbol* symbol = FindSymbol(name.data);
	if (symbol == NULL)
		UnknownSymbolError(name.data);
	
	ParseCall(symbol);
	if (symbol->data_type != DT_VOID)
		WriteLine("sdec 4");
	Match(";", TOKEN_LINE_END);
}

/* Parse an assign statement */
void ParseAssign()
{
	Token name = look;
	Match("Name", TOKEN_IDENTIFIER);
	
	/* Test for function call */
	if (look.id == TOKEN_OPEN_ARG)
	{
		ParseFunctionCall(name);
		return;
	}
	Match("=", TOKEN_ASSIGN);

	int type = ParseExpression();
	ParseStore(name.data, type);
	Match(";", TOKEN_LINE_END);
}

/* Parse a return statement */
void ParseReturn()
{
	/* Check for void return */
	Match("return", TOKEN_RETURN);
	int return_type = GetReturnType();
	if (return_type == DT_VOID)
	{
		WriteLine("return");
		Match(";", TOKEN_LINE_END);
		return;
	}
	
	/* Otherwise return a value */
	int type = ParseExpression();
	CorrectTypeing(return_type, type);
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

/* Parse a while loop */
void ParseWhile()
{
	Match("while", TOKEN_WHILE);
	char* start = GenerateLabel();
	char* end = GenerateLabel();

	Emit(start);
	ParseExpression();
	WriteLineVar("jumpifnot %s", end);
	ParseBlock();
	WriteLineVar("jump %s", start);
	Emit(end);

	free(start);
	free(end);
}

/* Parse a single statement */
void ParseStatement()
{
	switch(look.id)
	{
		case TOKEN_DECLARE: ParseDeclare(); break;
		case TOKEN_IDENTIFIER: ParseAssign(); break;
		case TOKEN_RETURN: ParseReturn(); break;
		case TOKEN_IF: ParseIf(); break;
		case TOKEN_WHILE: ParseWhile(); break;
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
