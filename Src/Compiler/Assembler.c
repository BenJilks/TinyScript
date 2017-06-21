#include "Assembler.h"
#include "Bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void AppendProgram(Program* program, int bytecode)
{
	program->bytecode[program->size++] = bytecode;
}

/* Returns a bytecode id from a name */
int GetBytecode(char* name)
{
	int i;
	for (i = 0; i < BC_COUNT; i++)
		if (!strcmp(bytecode_names[i], name))
			return i;
	return -1;
}

/* Returns a macro ID */
int GetMacroID(char macros[80][80], int macro_size, char* macro)
{
	int i;
	for (i = 0; i < macro_size; i++)
		if (!strcmp(macros[i], macro))
			return i;
	return -1;
}

/* Assemble source code into executable bytecode */
Program Assemble(char* source, int source_length)
{
	Program program;
	program.size = 0;

	/* Define macro data */
	char macro[80][80];
	int macro_data[80];
	int macro_size = 0;

	/* define macro assignments */
	int macro_location[80];
	char macro_name[80][80];
	int macro_assign_size = 0;

	int source_pointer = 0;
	while (source_pointer < source_length)
	{
		char instruction[80];
		sscanf(source + source_pointer, "%s", instruction);
		source_pointer += strlen(instruction) + 1;

		/* If the instruction is a macro definition */
		if (instruction[strlen(instruction) - 1] == ':')
		{
			memcpy(macro[macro_size], instruction, strlen(instruction) - 1);
			macro[macro_size][strlen(instruction) - 1] = '\0';
			macro_data[macro_size] = program.size;
			macro_size++;

			/* TEMP: define program starting point */
			if (!strcmp(instruction, "Main:"))
				program.start_pos = program.size;
			continue;
		}

		/* Write the instruction to the program */
		int bytecode = GetBytecode(instruction);
		AppendProgram(&program, bytecode);

		/* Read the arguments */
		int i;
		for (i = 0; i < bytecode_sizes[bytecode] - 1; i++)
		{
			char arg[80];
			sscanf(source + source_pointer, "%s", arg);
			source_pointer += strlen(arg) + 1;

			/* If the argument is a macro, then register it */
			if (isalpha(arg[0]))
			{
				macro_location[macro_assign_size] = program.size;
				memcpy(macro_name[macro_assign_size], arg, strlen(arg) + 1);
				macro_assign_size++;
				AppendProgram(&program, 0);
				continue;
			}
			AppendProgram(&program, atoi(arg));
		}
	}

	/* Link macros */
	int i;
	for (i = 0; i < macro_assign_size; i++)
		program.bytecode[macro_location[i]] = 
			macro_data[GetMacroID(macro, macro_size, macro_name[i])];

	return program;
}
