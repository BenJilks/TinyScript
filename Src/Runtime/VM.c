#include "VM.h"
#include "TinyScript.h"
#include "Bytecode.h"
#include "RuntimeLib.h"
#include <stdlib.h>
#include <memory.h>
#include <stdlib.h>

/* Argument creation functions */
Arguments EmptyArguments()
{
	Arguments args;
	args.size = 0;
	return args;
}
#define PARSE_ARGUMENT(args, arg, data_size) memcpy(args->data + args->size, &arg, data_size); args->size += data_size;
void ParseInt(Arguments* args, int i) { PARSE_ARGUMENT(args, i, 4); }
void ParseFloat(Arguments* args, float f) { PARSE_ARGUMENT(args, f, 4); }
void ParseChar(Arguments* args, char c) { PARSE_ARGUMENT(args, c, 1); }

#define DUMP_RAM(ram) \
{ \
	int i; for (i = 0; i < RAM_SIZE; i++) \
		printf("%i ", ram[i]); \
	printf("\n"); \
}

/* Define helper functions */
#define CPYINT(to, i) { int value = i; memcpy(ram + (to), (void*)&value, 4); }
#define CPYFLOAT(to, f) { float value = f; memcpy(ram + (to), (void*)&value, 4); }
#define POPINT(i) sp -= 4; memcpy((void*)&i, ram + sp, 4);
#define POPFLOAT(f) { int i; sp -= 4; memcpy((void*)&i, ram + sp, 4); (f) = *(float*)&i; }
#define VALUE rom[pc++]
#define OPERATION(op) { int l, r; POPINT(r); POPINT(l); CPYINT(sp, l op r); sp += 4; }
#define OPERATION_FLOAT(op) { float l, r; POPFLOAT(r); POPFLOAT(l); CPYFLOAT(sp, l op r); sp += 4; }

/* Calls a function within a compiled script */
CPUState CallFunction(Program program, char* func_name, Arguments arguments)
{
	/* Define storage type */
	int rom[PROGRAM_SIZE];
	char ram[RAM_SIZE];
	int pc = 0, sp = 0, fp = STACK_SIZE;
	
	/* Set the program starting point */
	Function func = GetFunction(program, func_name);
	pc = func.location;
	if (func.location == -1)
	{
		printf("Runtime Error: Could not find function with name '%s'\n", func_name);
		CPUState state;
		state.is_error_state = 1;
		return state;
	}

	/* Load data to RAM */
	memcpy(rom, program.bytecode, PROGRAM_SIZE * sizeof(int));
	memset(ram, 0, RAM_SIZE);
	CPYINT(sp, program.size);
	sp += 4;
	
	/* Parse the arguments */
	memcpy(ram + sp, arguments.data, arguments.size);
	sp += arguments.size;

	/* Interpret the program */
	int is_program_end = 0;
	while(pc < program.size && !is_program_end)
	{
		switch(rom[pc++])
		{
			/* Stack */
			case BC_PUSH: { CPYINT(sp, VALUE); sp += 4; } break;
			case BC_PUSHC: { memcpy(ram + sp, rom + (pc++), 1); sp += 1; } break;
			case BC_PUSHPC: { CPYINT(sp, pc); sp += 4; } break;
			case BC_SET: { sp -= 4; memcpy(ram + VALUE, ram + sp, 4); } break;
			case BC_GET: { memcpy(ram + sp, ram + VALUE, 4); sp += 4; } break;
			case BC_SETC: { sp -= 4; memcpy(ram + VALUE, ram + sp, 1); } break;
			case BC_GETC: { memset(ram + sp, 0, 4); memcpy(ram + sp, ram + VALUE, 1); sp += 4; } break;
			case BC_FSET: { sp -= 4; memcpy(ram + (fp + VALUE), ram + sp, 4); } break;
			case BC_FGET: { memcpy(ram + sp, ram + (fp + VALUE), 4); sp += 4; } break;
			case BC_FSETC: { sp -= 4; memcpy(ram + (fp + VALUE), ram + sp, 1); } break;
			case BC_FGETC: { memset(ram + sp, 0, 4); memcpy(ram + sp, ram + (fp + VALUE), 1); sp += 4; } break;
			
			/* Memory */
			case BC_CPY: { memcpy(ram + VALUE, ram + VALUE, VALUE); } break;
			case BC_SCPYTO: { memcpy(ram + sp, ram + VALUE, VALUE); } break;
			case BC_SCPYFROM: { memcpy(ram + VALUE, ram + sp, VALUE); } break;
			case BC_INC: { ram[rom[pc]] += rom[pc+1]; pc += 2; } break;
			case BC_DEC: { ram[rom[pc]] -= rom[pc+1]; pc += 2; } break;
			case BC_SINC: { sp += rom[pc++]; } break;
			case BC_SDEC: { sp -= rom[pc++]; } break;
			
			/* Stack Frame */
			case BC_SFCPYTO: { memcpy(ram + sp, ram + (rom[pc] + fp), rom[pc+1]); pc += 2; } break;
			case BC_SFCPYFROM: { memcpy(ram + (rom[pc] + fp), ram + sp, rom[pc+1]); pc += 2; } break;
			case BC_FINC: { fp += VALUE; } break;
			case BC_FDEC: { fp -= VALUE; } break;
			case BC_CPYARGS: { int size = VALUE; sp -= size; memcpy(ram + fp, ram + sp, size); } break;
			
			/* Maths */
			case BC_ADD: { OPERATION(+); } break;
			case BC_ADDF: { OPERATION_FLOAT(+); } break;
			case BC_SUB: { OPERATION(-); } break;
			case BC_SUBF: { OPERATION_FLOAT(-); } break;
			case BC_MUL: { OPERATION(*); } break;
			case BC_MULF: { OPERATION_FLOAT(*); } break;
			case BC_DIV: { OPERATION(/); } break;
			case BC_DIVF: { OPERATION_FLOAT(/); } break;
			
			/* Conditional */
			case BC_EQL: { OPERATION(==); } break;
			case BC_EQLF: { OPERATION_FLOAT(==); } break;
			case BC_NEQL: { OPERATION(!=); } break;
			case BC_NEQLF: { OPERATION_FLOAT(!=); } break;
			case BC_MORE: { OPERATION(>); } break;
			case BC_MOREF: { OPERATION_FLOAT(>); } break;
			case BC_LESS: { OPERATION(<); } break;
			case BC_LESSF: { OPERATION_FLOAT(<); } break;
			case BC_EMORE: { OPERATION(>=); } break;
			case BC_EMOREF: { OPERATION_FLOAT(>=); } break;
			case BC_ELESS: { OPERATION(<=); } break;
			case BC_ELESSF: { OPERATION_FLOAT(<=); } break;
			
			/* Program flow */
			case BC_JUMP: { pc = rom[pc]; } break;
			case BC_JUMPIFNOT: { pc++; int c; POPINT(c); if(!c) pc = rom[pc - 1]; } break;
			case BC_CALL: { CPYINT(sp, pc); sp += 4; pc = ram[pc]; } break;
			case BC_CCALL: { Lib_GetFunction(VALUE)(ram, &sp, &fp); } break;
			case BC_RETURN: { int addr; POPINT(addr); pc = addr; } break;
			case BC_IRETURN: { int i, addr; POPINT(i); POPINT(addr); pc = addr; CPYINT(sp, i); sp += 4; } break;
			
			/* Typeing */
			case BC_ITOF_LEFT: { int i; memcpy((void*)&i, ram + (sp-4), 4); float f = i; memcpy(ram + (sp-4), (void*)&f, 4); } break;
			case BC_ITOF_RIGHT: { int i; memcpy((void*)&i, ram + (sp-8), 4); float f = i; memcpy(ram + (sp-8), (void*)&f, 4); } break;
			case BC_FTOI: { float f; memcpy((void*)&f, ram + (sp-4), 4); int i = (int)f; memcpy(ram + (sp-4), (void*)&i, 4); } break;
		}
	}
	
	/* Return the ending state */
	CPUState state;
	memcpy(state.ram, ram, RAM_SIZE);
	state.pc = pc;
	state.sp = sp;
	state.fp = fp;
	state.is_error_state = 0;
	return state;
}

/* Runs a compiled program from the default starting point */
CPUState RunScript(Program program)
{
	return CallFunction(program, "Main", EmptyArguments());
}

/* Gets the returning value from the final state */
int GetReturnInt(CPUState state) { return *(int*)(state.ram + (state.sp - 4)); }
float GetReturnFloat(CPUState state) { return *(float*)(state.ram + (state.sp - 4)); }
char GetReturnChar(CPUState state) { return (char)GetReturnInt(state); }
