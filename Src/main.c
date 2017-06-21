#include <stdio.h>
#include "TinyScript.h"
#include "VM.h"

int main()
{
	InitIO("in.txt");
	InitParser();

	/* Compile the code */
	ParseProgramBody();
	printf("%s\n", GetOutput());
	
	/* Assemble and run the code */
	Program program = Assemble(GetOutput(), GetOutputSize());
	RunScript(program);
	CloseIO();
	return 0;
}
