#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TinyScript.h"
#include "VM.h"
#include "BinFile.h"
#include "../Lib/TinyLib.h"

/* Reports an error state */
void ErrorNoInput()
{
	printf("Error: No input file given (use '--help' for more information)\n");
	exit(0);
}
void UnkownArgument(char* arg)
{
	printf("Error: Unkown argument '%s' (use '--help' for more information)\n", arg);
	exit(0);
}
void ConflictError()
{
	printf("Error: Conflicting options  (use '--help' for more information)\n");
	exit(0);
}

/* Prints the help screen */
void PrintHelp()
{
	printf("Usage:\nTinyScript <input file> [options]\n\nOptions:\n"
			"	--help Prints out the help information\n"
			"	-v     Prints version data\n"
			"	-bin   Runs the input file as a compiled TinyScript bin file\n"
			"	-S     Only compile to assembly and outputs to a file\n"
			"	-o     Outputs the compiled program to a bin file\n"
	);
	exit(0);
}

/* Prints the version number */
void PrintVersion()
{
	printf("TinyScript: %s\n", VERSION);
	exit(0);
}

/* Outputs a program to a bin file */
void OutputBin(Program program, char* file_path)
{
	int i;
	BinFile_Open(file_path, "wb");
	
	/* Write function data */
	BinFile_WriteInt(program.function_size);
	for (i = 0; i < program.function_size; i++)
	{
		Function func = program.functions[i];
		BinFile_WriteString(func.name);
		BinFile_WriteInt(func.location);
	}
	
	/* Write program data */
	BinFile_WriteInt(program.size);
	BinFile_WriteData((char*)program.bytecode, program.size * sizeof(int));
	BinFile_Close();
}

/* Reads a program from a bin file */
Program ReadBin(char* file_path)
{
	BinFile_Open(file_path, "rb");
	Program program;
	
	int i, func_size = BinFile_ReadInt();
	for (i = 0; i < func_size; i++)
	{
		Function func;
		BinFile_ReadString(func.name);
		func.location = BinFile_ReadInt();
		program.functions[program.function_size++] = func;
	}
	
	program.size = BinFile_ReadInt();
	BinFile_ReadData((char*)program.bytecode, program.size * sizeof(int));
	BinFile_Close();
	return program;
}

/* Define compile modes */
#define MODE_SCRIPT          0
#define MODE_RUN_BIN         1
#define MODE_OUTPUT_ASSEMBLY 2
#define MODE_OUTPUT_BIN      3

int main(int argc, char* argv[])
{
	char* input = NULL;
	int compiler_mode = MODE_SCRIPT;
	
	/* Read arguments */
	int index = 1;
	while (index < argc)
	{
		if (!strcmp(argv[index], "--help")) PrintHelp();
		else if (!strcmp(argv[index], "-v")) PrintVersion();
		else if (!strcmp(argv[index], "-bin")) compiler_mode = MODE_RUN_BIN;
		else if (!strcmp(argv[index], "-S")) compiler_mode = MODE_OUTPUT_ASSEMBLY;
		else if (!strcmp(argv[index], "-o")) compiler_mode = MODE_OUTPUT_BIN;
		else
		{
			if (input != NULL)
				UnkownArgument(argv[index]);
			input = argv[index];
		}
		index++;
	}
	if (input == NULL) ErrorNoInput();
	
	InitIO(input);
	AddTinyLib();
	Program program;
	switch(compiler_mode)
	{
		case MODE_SCRIPT:
			InitParser();
			ParseProgramBody();
			program = Assemble(GetOutput(), GetOutputSize());
			RunScript(program);
			break;
		
		case MODE_RUN_BIN:
			program = ReadBin(input);
			RunScript(program);
			break;
		
		case MODE_OUTPUT_ASSEMBLY:
			InitParser();
			ParseProgramBody();
			WriteOutputToFile("out.s");
			break;
		
		case MODE_OUTPUT_BIN:
			InitParser();
			ParseProgramBody();
			program = Assemble(GetOutput(), GetOutputSize());
			OutputBin(program, "out.bin");
			break;
	}
	CloseIO();
	return 0;
}
