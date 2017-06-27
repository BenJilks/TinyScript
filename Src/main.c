#include <stdio.h>
#include "TinyScript.h"
#include "VM.h"

int main()
{
	InitIO("in.txt");
	InitParser();

	/* Compile the code */
	ParseProgramBody();
	//printf("\n%s\n", GetOutput());
	
	/* Assemble and run the code */
	Program program = Assemble(GetOutput(), GetOutputSize());
	
	int i, x = 0;
	CPUState state;
	for (i = 0; i < 10; i++) 
	{
		Arguments args = EmptyArguments();
		ParseInt(&args, x);
		state = CallFunction(program, "f", args);
		x = GetReturnInt(state);
		
		printf("%i\n", x);
	}
	CloseIO();
	return 0;
}
