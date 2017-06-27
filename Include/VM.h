#ifndef VM
#define VM

#include "Assembler.h"
#include "Config.h"

/* Store the state of the program once execution has finished */
typedef struct CPUState
{
	int ram[RAM_SIZE];
	int pc, sp, fp;
	int is_error_state;
} CPUState;

/* Store the arguments to be parsed onto the function */
typedef struct Arguments
{
	char data[80];
	int size;
} Arguments;
Arguments EmptyArguments();
void ParseInt(Arguments* arguments, int i);
void ParseFloat(Arguments* arguments, float f);
void ParseChar(Arguments* arguments, char c);

/* Calls a function within a compiled script */
CPUState CallFunction(Program program, char* func_name, Arguments arguments);

/* Runs a compiled program from the default starting point */
CPUState RunScript(Program program);

/* Gets the returning value from the final state */
int GetReturnInt(CPUState state);
float GetReturnFloat(CPUState state);
char GetReturnChar(CPUState state);

#endif
