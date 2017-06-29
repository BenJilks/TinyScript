#include "TinyScript.h"
#include <stdlib.h>
#include <memory.h>

/* Parse a global declare statement */
void ParseGlobalDeclare()
{
	Match("declare", TOKEN_DECLARE);
	Token type = look;
	Match("Type", TOKEN_IDENTIFIER);

	/* For each veriable name, create a new global variable */
	while(1)
	{
		Token name = look;
		CreateGlobalVariable(name.data, type.data);
		Match("Name", TOKEN_IDENTIFIER);

		if (look.id != TOKEN_LIST)
			break;
		Match(",", TOKEN_LIST);
	}
	Match(";", TOKEN_LINE_END);
}

/* Generate the code of the start of a function */
void FunctionHeader(char* name)
{
	WriteLineVar("%s:", name);
	StartStackFrame();
}

/* Reads the paramitor list */
void ReadParams(Symbol** params, int* size)
{
	Match("(", TOKEN_OPEN_ARG);
	while (look.id != TOKEN_CLOSE_ARG)
	{
		/* Get parameter data */
		Token type = look;
		Match("Type", TOKEN_IDENTIFIER);
		Token name = look;
		Match("Name", TOKEN_IDENTIFIER);

		/* Create local variables */
		Symbol* symbol = CreateLocalVariable(name.data, type.data);
		params[(*size)++] = symbol;

		if (look.id == TOKEN_LIST)
			Match(",", TOKEN_LIST);
	}
	Match(")", TOKEN_CLOSE_ARG);
}

/* Generate stack to stack frame copying code */
void InitStackFrame(Symbol* params[80], int size)
{
	int i, data_size = 0;
	for (i = size - 1; i >= 0; i--)
		data_size += TypeSize(params[i]->data_type);
	
	WriteLineVar("cpyargs %i", data_size);
}

/* Parse a function declaration statement */
void ParseFunction()
{
	/* Init function header data */
	Match("function", TOKEN_FUNCTION);
	Token name = look;
	Match("Name", TOKEN_IDENTIFIER);
	FunctionHeader(name.data);
	
	/* Read parameter list */
	Symbol* params[80];
	int param_size = 0;
	StartScope();
	ReadParams(params, &param_size);

	Token type = CreateToken("void", 5, TOKEN_IDENTIFIER);
	if (look.id == TOKEN_LESS)
	{
		Match("<", TOKEN_LESS);
		type = look;
		Match("Type", TOKEN_IDENTIFIER);
	}
	SetReturnType(type.data);
	
	/* Parse the function code */
	Symbol* func = CreateFunction(name.data, type.data, params, param_size);
	
	/* Make a copy of the function */
	Symbol* funccpy = (Symbol*)malloc(sizeof(Symbol));
	memcpy(funccpy, func, sizeof(Symbol));
	
	if (param_size > 0)
		InitStackFrame(params, param_size);
	ParseBlock();
	PopScope();
	
	/* Reregister the function for later use */
	RegisterSymbol(funccpy);
	
	/* Add defualt return statement */
	WriteLine("return");
}

/* Parse a program body */
void ParseProgramBody()
{
	while (look.id != TOKEN_NULL)
	{
		switch(look.id)
		{
			case TOKEN_INCLUDE: break;
			case TOKEN_DECLARE: ParseGlobalDeclare(); break;
			case TOKEN_FUNCTION: ParseFunction(); break;
			default:
			{
				char msg[80];
				sprintf(msg, "Unexpected token '%s'", look.data);
				Abort(msg);
			}
		}
	}
}
