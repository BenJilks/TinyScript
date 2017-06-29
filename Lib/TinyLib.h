#ifndef TINYLIB
#define TINYLIB

#include "RuntimeLib.h"
#include <stdio.h>

static void DebugFunc(char* ram, int* sp, int* fp)
{
	printf("Running debug function...\n");
	printf("Parsed argument: %i\n", ram[(*sp) - 4]);
	printf("Echo back argument\n");
}

static void AddTinyLib()
{
	Lib_RegisterFunction("DebugFunc", DebugFunc, DT_INT);
}

#endif
