#ifndef ASSEMBLER
#define ASSEMBLER

/* Store the compiled program */
#define PROGRAM_SIZE 80
typedef struct Program
{
	int bytecode[PROGRAM_SIZE];
	int size;
	int start_pos;
} Program;

/* Assemble source code into executable bytecode */
Program Assemble(char* source, int source_length);

#endif
