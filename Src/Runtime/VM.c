#include "VM.h"
#include "TinyScript.h"
#include "Bytecode.h"
#include "Config.h"
#include <stdlib.h>
#include <memory.h>
#include <stdlib.h>

#define DUMP_RAM(ram) \
{ \
	int i; for (i = 0; i < RAM_SIZE; i++) \
		printf("%i ", ram[i]); \
	printf("\n"); \
}

/* Define helper functions */
#define CPYINT(to, i) { int value = i; memcpy(ram + (to), (void*)&value, 4); }
#define POPINT(i) sp -= 4; memcpy((void*)&i, ram + sp, 4);
#define VALUE rom[pc++] 
#define OPERATION(op) { int l, r; POPINT(r); POPINT(l); CPYINT(sp, l op r); sp += 4; }

/* Runs a compiled program */
void RunScript(Program program)
{
	/* Define storage type */
	int rom[PROGRAM_SIZE];
	char ram[RAM_SIZE];
	int pc = program.start_pos, sp = 0, fp = STACK_SIZE;

	/* Load data to RAM */
	memcpy(rom, program.bytecode, PROGRAM_SIZE * sizeof(int));
	memset(ram, 0, RAM_SIZE);
	CPYINT(sp, program.size);
	sp += 4;

	/* Interpret the program */
	int is_program_end = 0;
	while(pc < program.size && !is_program_end)
	{
		//printf("%i) %s\n", pc, bytecode_names[rom[pc]]);
		switch(rom[pc++])
		{
			/* Stack */
			case BC_PUSH: { CPYINT(sp, VALUE); sp += 4; } break;
			case BC_PUSHPC: { CPYINT(sp, pc); sp += 4; } break;
			case BC_SET: { sp -= 4; memcpy(ram + VALUE, ram + sp, 4); } break;
			case BC_GET: { memcpy(ram + sp, ram + VALUE, 4); sp += 4; } break;
			case BC_FSET: { sp -= 4; memcpy(ram + (fp + VALUE), ram + sp, 4); } break;
			case BC_FGET: { memcpy(ram + sp, ram + (fp + VALUE), 4); sp += 4; } break;

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
			
			/* Maths */
			case BC_ADD: { OPERATION(+); } break;
			case BC_SUB: { OPERATION(-); } break;
			case BC_MUL: { OPERATION(*); } break;
			case BC_DIV: { OPERATION(/); } break;
			
			/* Conditional */
			case BC_EQL: { OPERATION(==); } break;
			case BC_NEQL: { OPERATION(!=); } break;
			case BC_MORE: { OPERATION(>); } break;
			case BC_LESS: { OPERATION(<); } break;
			case BC_EMORE: { OPERATION(>=); } break;
			case BC_ELESS: { OPERATION(<=); } break;
			
			/* Program flow */
			case BC_JUMP: { pc = rom[pc]; } break;
			case BC_JUMPIFNOT: { pc++; int c; POPINT(c); if(!c) pc = rom[pc - 1]; } break;
			case BC_CALL: { CPYINT(sp, pc); sp += 4; pc = ram[pc]; } break;
			case BC_RETURN: { int addr; POPINT(addr); pc = addr; CPYINT(sp, 0); sp += 4; } break;
			case BC_IRETURN: { int i, addr; POPINT(i); POPINT(addr); pc = addr; CPYINT(sp, i); sp += 4; } break;
		}
	}
	
	/* Debug print the output */
	printf("%i, %i\n", 
		   *(int*)(ram + (STACK_SIZE + FRAME_SIZE)), 
		   *(int*)(ram + (STACK_SIZE + FRAME_SIZE + 4)));
}
