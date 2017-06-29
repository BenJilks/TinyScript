#include "TinyScript.h"
#include <stdlib.h>
#include <memory.h>

/* Define symbol table */
Symbol* table[80];
int table_pointer = 0;

/* Define scope stack */
int scope_stack[80];
int scope_stack_pointer = 0;

/* Allocate memory for a new symbol and return it */
Symbol* CreateSymbol(char* name, int type)
{
	Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
	memcpy(symbol->name, name, strlen(name) + 1);
	symbol->type = type;
	return symbol;
}

/* Frees all the data in the symbol table */
void WipeTableClean()
{
	int i;
	for (i = 0; i < table_pointer; i++)
		free(table[i]);
	table_pointer = 0;
}

/* Find a symbol in the symbol table */
Symbol* FindSymbol(char* name)
{
	int i;
	for (i = table_pointer - 1; i >= 0; i--)
		if (!strcmp(name, table[i]->name))
			return table[i];
	return NULL;
}

/* Start a new scope */
void StartScope()
{
	scope_stack[scope_stack_pointer++] = 0;
}

/* Register a new symbol in the symbol table under the current scope */
void RegisterSymbol(Symbol* symbol)
{
	table[table_pointer++] = symbol;
	scope_stack[scope_stack_pointer - 1]++;
}

/* Clear all the symbols in the current scope */
void PopScope()
{
	/* Get the current scope size */
	int scope_size = scope_stack[scope_stack_pointer - 1];

	/* Free all the data in that scope */
	int i;
	for (i = 0; i < scope_size; i++)
		free(table[table_pointer - 1 - i]);
	table_pointer -= scope_size;
	scope_stack_pointer--;
}

/* Generates a new label */
#define LABEL_SIZE 10
int lable_id = 0;
char* GenerateLabel()
{
	char* label = (char*)malloc(LABEL_SIZE);
	sprintf(label, "L%i", lable_id++);
	return label;
}

/* Emits a label to define its adress */
void Emit(char* label)
{
	WriteLineVar("%s:", label);
}
