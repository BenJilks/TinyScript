#include "TinyScript.h"
#include <stdio.h>
#include <stdlib.h>

/* Prints an error message to the screen */
void Error(char* msg)
{
	printf("Error, line %i: %s\n", GetLineCount(), msg);
}

/* Prints an error message then quits */
void Abort(char* msg)
{
	Error(msg);
	WipeTableClean();
	CloseIO();
	exit(0);
}

/* Reports on an unknown symbol error */
void UnknownSymbolError(char* symbol)
{
	char msg[80];
	sprintf(msg, "The symbol '%s' has not been defined", symbol);
	Abort(msg);
}
