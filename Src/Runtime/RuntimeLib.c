#include "RuntimeLib.h"
#include <string.h>
#include <stdio.h>

/* Store registering data */
CFunction functions[FUNC_SIZE];
char function_names[FUNC_SIZE][80];
int function_return[FUNC_SIZE];
int func_id_pointer = 0;

/* Register a new function to the library */
void ResetLib() { func_id_pointer = 0; }
int Lib_RegisterFunction(char* name, CFunction func, int ret)
{
	int id = func_id_pointer++;
	functions[id] = func;
	memcpy(function_names[id], name, strlen(name) + 1);
	function_return[id] = ret;
	return id;
}

/* Get functions by there ID */
CFunction Lib_GetFunction(int id) { return functions[id]; }
int Lib_GetReturn(int id) { return function_return[id]; }
int Lib_GetFunctionIdByName(char* name)
{
	int i;
	for (i = 0; i < func_id_pointer; i++)
		if (!strcmp(function_names[i], name))
			return i;
	return -1;
}
