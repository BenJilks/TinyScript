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
	CloseIO();
	exit(0);
}
