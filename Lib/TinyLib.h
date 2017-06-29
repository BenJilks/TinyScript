#ifndef TINYLIB
#define TINYLIB

#include "RuntimeLib.h"
#include <stdio.h>
#define FUNC(name) static void name(char* ram, int* sp, int* fp)

FUNC(DebugFunc)
{
	printf("Running debug function...\n");
	printf("Parsed argument: %i\n", *(int*)(ram + ((*sp) - 4)));
	printf("Echo back argument\n");
}

FUNC(PrintInt)
{
	(*sp) -= 4;
	printf("%i\n", *(int*)(ram + (*sp)));
}

static void AddTinyLib()
{
	Lib_RegisterFunction("DebugFunc", DebugFunc, DT_INT);
	Lib_RegisterFunction("PrintInt", PrintInt, DT_VOID);
}

#endif
