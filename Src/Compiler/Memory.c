#include "TinyScript.h"
#include "Config.h"
#include <string.h>
#include <stdlib.h>

/* Define static memory pointer */
int memory_pointer = STACK_SIZE + FRAME_SIZE;
int local_pointer = 0;

/* Converts a string type into a type ID */
int GetTypeID(char* name)
{
	static char* data_type_names[] = {"int", "float", "char"};

	int i;
	for (i = 0; i < DT_SIZE; i++)
		if (!strcmp(name, data_type_names[i]))
			return i;
	return -1;
}

/* Returns the size of a type */
int TypeSize(int type)
{
	switch(type)
	{
		case DT_INT: return 4;
		case DT_FLOAT: return 4;
		case DT_CHAR: return 1;
	}
	return 0;
}

/* Allocates a new location in static memory */
int Allocate(int size)
{
	int pointer = memory_pointer;
	memory_pointer += size;
	return pointer;
}

/* Sets up data for a new stack frame */
void StartStackFrame()
{
	local_pointer = 0;
}

/* Creates, allocates and registers a new global variable */
Symbol* CreateGlobalVariable(char* name, char* type)
{
	Symbol* symbol = CreateSymbol(name, SYMBOL_GLOBAL_VARIABLE);
	symbol->data_type = GetTypeID(type);
	symbol->location = Allocate(TypeSize(symbol->data_type));
	RegisterSymbol(symbol);
	return symbol;
}

/* Creates a new local variable */
Symbol* CreateLocalVariable(char* name, char* type)
{
	Symbol* symbol = CreateSymbol(name, SYMBOL_LOCAL_VARIABLE);
	symbol->data_type = GetTypeID(type);
	symbol->location = local_pointer;
	RegisterSymbol(symbol);

	local_pointer += TypeSize(symbol->data_type);
	return symbol;
}

/* Creates a new function */
void CreateFunction(char* name, char* type)
{
	Symbol* symbol = CreateSymbol(name, SYMBOL_FUNCTION);
	symbol->data_type = GetTypeID(type);
	RegisterSymbol(symbol);
}

/* Calls a functions */
void ParseCall(Symbol* symbol)
{
	/* Move to the next stack frame */
	char* label = GenerateLabel();
	WriteLineVar("push %s", label);
	
	/* Parse the arguments and call the function */
	Match("(", TOKEN_OPEN_ARG);
	while (look.id != TOKEN_CLOSE_ARG)
	{
		ParseExpression();
		if (look.id == TOKEN_LIST)
			Match(",", TOKEN_LIST);
	}
	Match(")", TOKEN_CLOSE_ARG);
	WriteLineVar("finc %i", local_pointer);
	WriteLineVar("jump %s", symbol->name);
	Emit(label);
	free(label);
	
	/* Clean the stack frame */
	WriteLineVar("fdec %i", local_pointer);
}

/* Reports on an unknown symbol error */
void UnknownSymbolError(char* symbol)
{
	char msg[80];
	sprintf(msg, "The symbol '%s' has not been defined", symbol);
	Abort(msg);
}

/* Loads a variable from memory */
void ParseLoad()
{
	/* Get the variable name */
	Token name = look;
	Match("Name", TOKEN_IDENTIFIER);

	/* Get the symbol */
	Symbol* symbol = FindSymbol(name.data);
	if (symbol == NULL)
		UnknownSymbolError(name.data);

	/* If the variable is a function call, then call it */
	if (symbol->type == SYMBOL_FUNCTION)
	{
		ParseCall(symbol);
		return;
	}

	/* Otherwise load the variable to the stack */
	if (symbol->type == SYMBOL_LOCAL_VARIABLE)
	{
		WriteLineVar("fget %i", symbol->location);
		return;
	}
	WriteLineVar("get %i", symbol->location);
}

/* Stores a variable to memory */
void ParseStore(char* name)
{
	/* Get the symbol */
	Symbol* symbol = FindSymbol(name);
	if (symbol == NULL)
		UnknownSymbolError(name);

	/* Store the variable from the stack to memory */
	if (symbol->type == SYMBOL_LOCAL_VARIABLE)
	{
		WriteLineVar("fset %i", symbol->location);
		return;
	}
	WriteLineVar("set %i", symbol->location);
}
