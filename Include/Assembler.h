#ifndef ASSEMBLER
#define ASSEMBLER

/* Store data about a single function */
typedef struct Function
{
	char name[80];
	int location;
} Function;

/* Store the compiled program */
#define PROGRAM_SIZE 80
typedef struct Program
{
	/* Program data */
	int bytecode[PROGRAM_SIZE];
	int size;
	
	/* Function data */
	Function functions[80];
	int function_size;
} Program;

/* Returns the location of a function by its name */
Function GetFunction(Program program, char* name);

/* Assemble source code into executable bytecode */
Program Assemble(char* source, int source_length);

#endif
