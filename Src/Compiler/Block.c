#include "TinyScript.h"
#include "RuntimeLib.h"
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
	int cfunc = Lib_GetFunctionIdByName(name.data);
	int data_size = 0;
	if (cfunc != -1)
	{
		/* Call C function */
		int type = CallCFunction(cfunc);
		data_size = TypeSize(type);
	}
	else
	{
		/* Call bytecode function */
		Symbol* symbol = FindSymbol(name.data);
		if (symbol == NULL)
			UnknownSymbolError(name.data);
		
		ParseCall(symbol);
		if (symbol->data_type != DT_VOID)
			WriteLine("sdec 4");
		data_size = TypeSize(symbol->data_type);
	}
	
	/* Pop the returning value */
	if (data_size > 0)
		WriteLineVar("sdec %i", data_size);
}

/* Parse an assign statement */
void ParseAssignOrCall()
{
	Token name = look;
	Match("Name", TOKEN_IDENTIFIER);
	
	if (look.id == TOKEN_OPEN_ARG)
	{
		/* Parse function call */
		ParseFunctionCall(name);
	}
	else
	{
		/* Parse assign statement */
		Match("=", TOKEN_ASSIGN);
		int type = ParseExpression();
		ParseStore(name.data, type);
	}
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
	
	StartScope();
	char* label = GenerateLabel();
	WriteLineVar("jumpifnot %s", label);
	ParseBlock();
	PopScope();
	
	Emit(label);
	free(label);
}

/* Parse a while loop */
void ParseWhile()
{
	Match("while", TOKEN_WHILE);
	char* start = GenerateLabel();
	char* end = GenerateLabel();

	StartScope();
	Emit(start);
	ParseExpression();
	WriteLineVar("jumpifnot %s", end);
	ParseBlock();
	WriteLineVar("jump %s", start);
	Emit(end);
	PopScope();

	free(start);
	free(end);
}

/* Parse a single statement */
void ParseStatement()
{
	switch(look.id)
	{
		case TOKEN_DECLARE: ParseDeclare(); break;
		case TOKEN_IDENTIFIER: ParseAssignOrCall(); break;
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
