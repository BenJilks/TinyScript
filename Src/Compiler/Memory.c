#include "TinyScript.h"
#include "Config.h"
#include <string.h>
#include <stdlib.h>

/* Define static memory pointer */
int memory_pointer = STACK_SIZE + FRAME_SIZE;
int local_pointer = 0;
int return_type = -1;

/* Converts a string type into a type ID */
int GetTypeID(char* name)
{
	static char* data_type_names[] = {"int", "float", "char", "void"};

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
		case DT_VOID: return 0;
	}
	return 0;
}

/* Handles an unkown type error */
void UnkownType(char* type)
{
	char msg[80];
	sprintf(msg, "Unkown type name '%s'", type);
	Abort(msg);
}

/* Allocates a new location in static memory */
int Allocate(int size)
{
	int pointer = memory_pointer;
	memory_pointer += size;
	return pointer;
}

/* Sets up data for a new stack frame */
void StartStackFrame(char* type)
{
	local_pointer = 0;
	return_type = GetTypeID(type);
}

/* Creates, allocates and registers a new global variable */
Symbol* CreateGlobalVariable(char* name, char* type)
{
	Symbol* symbol = CreateSymbol(name, SYMBOL_GLOBAL_VARIABLE);
	symbol->data_type = GetTypeID(type);
	symbol->location = Allocate(TypeSize(symbol->data_type));
	RegisterSymbol(symbol);
	
	if (symbol->data_type == -1)
		UnkownType(type);
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
	if (symbol->data_type == -1)
		UnkownType(type);
	return symbol;
}

/* Creates a new function */
Symbol* CreateFunction(char* name, char* type, Symbol* params[80], int param_size)
{
	int i;
	Symbol* symbol = CreateSymbol(name, SYMBOL_FUNCTION);
	symbol->data_type = GetTypeID(type);
	symbol->param_size = param_size;
	for (i = 0; i < param_size; i++)
		symbol->params[i] = params[i]->data_type;
	RegisterSymbol(symbol);
	
	if (symbol->data_type == -1)
		UnkownType(type);
	return symbol;
}

/* Parse the arguments of a function call */
void ParseArguments(Symbol* symbol)
{
	int index = 0;
	Match("(", TOKEN_OPEN_ARG);
	while (look.id != TOKEN_CLOSE_ARG)
	{
		int param_type = symbol->params[index];
		int type = ParseExpression();
		if (param_type == DT_CHAR) 
			WriteLine("sdec 3");
		
		/* Check argument */
		if (index >= symbol->param_size)
			Abort("Too many arguments in function call");
		CorrectTypeing(param_type, type);
		index++;
		
		if (look.id == TOKEN_LIST)
			Match(",", TOKEN_LIST);
	}
	Match(")", TOKEN_CLOSE_ARG);
	
	if (index < symbol->param_size)
		Abort("Too few arguments in function call");
}

/* Calls a functions */
void ParseCall(Symbol* symbol)
{
	//WriteLineVar("\n; Function call to '%s'", symbol->name);
	
	/* Move to the next stack frame */
	char* label = GenerateLabel();
	WriteLineVar("push %s", label);
	
	/* Parse the arguments and call the function */
	ParseArguments(symbol);
	if (local_pointer > 0)
		WriteLineVar("finc %i", local_pointer);
	WriteLineVar("jump %s", symbol->name);
	Emit(label);
	free(label);
	
	/* Clean the stack frame */
	if (local_pointer > 0)
		WriteLineVar("fdec %i", local_pointer);
	//WriteLine("");
}

/* Loads a variable from memory */
int ParseLoad()
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
		return symbol->data_type;
	}

	/* Otherwise load the variable to the stack */
	char* type_string = symbol->data_type == DT_CHAR ? "c" : "";
	if (symbol->type == SYMBOL_LOCAL_VARIABLE)
	{
		WriteLineVar("fget%s %i", type_string, symbol->location);
		return symbol->data_type;
	}
	WriteLineVar("get%s %i", type_string, symbol->location);
	return symbol->data_type;
}

/* TYPE_CHECK_ID */
#define TCI(ltype, rtype) (rtype) * DT_SIZE + (ltype)

/* Makes shore that the data being set is of the correct type */
void CorrectTypeing(int aim, int type)
{
	if (aim == DT_VOID)
		Abort("Cannot assign 'void' type a value");
	
	switch(TCI(aim, type))
	{
		case TCI(DT_INT, DT_FLOAT):
		case TCI(DT_CHAR, DT_FLOAT):
			WriteLine("ftoi");
			break;
		
		case TCI(DT_FLOAT, DT_INT):
		case TCI(DT_FLOAT, DT_CHAR):
			WriteLine("itofleft");
			break;
	}
}

/* Stores a variable to memory */
int ParseStore(char* name, int type)
{
	/* Get the symbol */
	Symbol* symbol = FindSymbol(name);
	if (symbol == NULL)
		UnknownSymbolError(name);
	CorrectTypeing(symbol->data_type, type);
	
	/* Store the variable from the stack to memory */
	char* type_string = symbol->data_type == DT_CHAR ? "c" : "";
	if (symbol->type == SYMBOL_LOCAL_VARIABLE)
	{
		WriteLineVar("fset%s %i", type_string, symbol->location);
		return symbol->data_type;
	}
	WriteLineVar("set%s %i", type_string, symbol->location);
	return symbol->data_type;
}

/* Gets the current return type */
int GetReturnType()
{
	return return_type;
}
