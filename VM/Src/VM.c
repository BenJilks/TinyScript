#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VM.h"

#define DEBUG 0
#define LOG_STACK 0

#if DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

typedef enum ByteCode
{
	ALLOC = 0,
	PUSH_ARG,
	PUSH_LOC,
	PUSH_INT,
	PUSH_FLOAT,
	PUSH_CHAR,
	PUSH_BOOL,
	PUSH_STRING,
	POP,
	ASSIGN,
	ADD,
	SUB,
	MUL,
	DIV,
	EQUALS,
	GREATERTHAN,
	LESSTHAN,
	CALL,
	CALL_SYS,
	RETURN,
	BRANCH_IF_NOT,
	BRANCH,
	INC_LOC,
	POP_ARGS,
	MALLOC,
	PUSH_ATTR,
	ASSIGN_ATTR
} ByteCode;

Type t_int = {"int", INT, 4};
Type t_float = {"float", FLOAT, 4};
Type t_string = {"string", STRING, 8};
Type t_bool = {"bool", BOOL, 1};
Type t_char = {"char", CHAR, 1};

Type *PrimType(int prim)
{
	switch(prim)
	{
		case INT: return &t_int;
		case FLOAT: return &t_float;
		case STRING: return &t_string;
		case BOOL: return &t_bool;
		case CHAR: return &t_char;
	}
	return NULL;
}

#define ERROR(...) \
	printf(__VA_ARGS__); \
	exit(0)

#define OP_FUNC(name, op) \
	Object name(Object left, Object right) \
	{ \
		Object obj; \
		switch(left.type->prim) \
		{ \
			case INT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_int; obj.i = left.i op right.i; break; \
					case FLOAT: obj.type = &t_float; obj.f = left.i op right.f; break; \
					case CHAR: obj.type = &t_int; obj.i = left.i op right.c; break; \
					default: ERROR("Invalid operation"); break; \
				} \
				break; \
			case FLOAT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_float; obj.f = left.f op right.i; break; \
					case FLOAT: obj.type = &t_float; obj.f = left.f op right.f; break; \
					case CHAR: obj.type = &t_float; obj.f = left.f op right.c; break; \
					default: ERROR("Invalid operation"); break; \
				} \
				break; \
			case CHAR: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_int; obj.i = left.c op right.i; break; \
					case FLOAT: obj.type = &t_float; obj.f = left.c op right.f; break; \
					case CHAR: obj.type = &t_char; obj.c = left.c op right.c; break; \
					default: ERROR("Invalid operation"); break; \
				} \
				break; \
			case BOOL: \
				ERROR("Invalid operation"); \
				break; \
		} \
		return obj; \
	}

#define COMPARE_FUNC(name, op) \
	Object name(Object left, Object right) \
	{ \
		Object obj; \
		switch(left.type->prim) \
		{ \
			case INT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_bool; obj.i = left.i op right.i; break; \
					case FLOAT: obj.type = &t_bool; obj.i = left.i op right.f; break; \
					case CHAR: obj.type = &t_bool; obj.i = left.i op right.c; break; \
					default: ERROR("Invalid operation"); break; \
				} \
				break; \
			case FLOAT: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_bool; obj.i = left.f op right.i; break; \
					case FLOAT: obj.type = &t_bool; obj.i = left.f op right.f; break; \
					case CHAR: obj.type = &t_bool; obj.i = left.f op right.c; break; \
					default: ERROR("Invalid operation"); break; \
				} \
				break; \
			case CHAR: \
				switch(right.type->prim) \
				{ \
					case INT: obj.type = &t_bool; obj.i = left.c op right.i; break; \
					case FLOAT: obj.type = &t_bool; obj.i = left.c op right.f; break; \
					case CHAR: obj.type = &t_bool; obj.i = left.c op right.c; break; \
					default: ERROR("Invalid operation"); break; \
				} \
				break; \
			case BOOL: \
				ERROR("Invalid operation"); \
				break; \
		} \
		return obj; \
	}

Object Equals(Object left, Object right)
{
	Object obj;
	switch(left.type->prim)
	{
		case INT:
			switch(right.type->prim)
			{
				case INT: obj.type = &t_bool; obj.i = left.i == right.i; break;
				case FLOAT: obj.type = &t_bool; obj.i = left.i == right.f; break;
				case CHAR: obj.type = &t_bool; obj.i = left.i == right.c; break;
				default: ERROR("Invalid operation"); break;
			}
			break;
		case FLOAT:
			switch(right.type->prim)
			{
				case INT: obj.type = &t_bool; obj.i = left.f == right.i; break;
				case FLOAT: obj.type = &t_bool; obj.i = left.f == right.f; break;
				case CHAR: obj.type = &t_bool; obj.i = left.f == right.c; break;
				default: ERROR("Invalid operation"); break;
			}
			break;
		case CHAR:
			switch(right.type->prim)
			{
				case INT: obj.type = &t_bool; obj.i = left.c == right.i; break;
				case FLOAT: obj.type = &t_bool; obj.i = left.c == right.f; break;
				case CHAR: obj.type = &t_bool; obj.i = left.c == right.c; break;
				default: ERROR("Invalid operation"); break;
			}
			break;
		case BOOL:
			switch(right.type->prim)
			{
				case BOOL: obj.type = &t_bool; obj.i = left.c == right.c; break;
				default: ERROR("Invalid operation"); break;
			}
			break;
		case STRING:
			switch(right.type->prim)
			{
				case STRING: obj.type = &t_bool; obj.i = !strcmp((char*)left.p, (char*)right.p); break;
				default: ERROR("Invalid operation"); break;
			}
			break;
	}
	return obj;
}

OP_FUNC(Add, +);
OP_FUNC(Sub, -);
OP_FUNC(Mul, *);
OP_FUNC(Div, /);
COMPARE_FUNC(GreaterThan, >);
COMPARE_FUNC(LessThan, <);

#define OP_INSTUCTION(code, op) \
	case code: \
		LOG("%s operation\n", #op); \
		stack[(--sp)-1] = op(stack[sp-2], stack[sp-1]); \
		break; \

typedef struct SysCall
{
	char name[80];
	SysFunc func;
} SysCall;

SysCall sys_calls[80];
int sys_call_size = 0;

void RegisterFunc(char *name, SysFunc func)
{
	SysCall call;
	strcpy(call.name, name);
	call.func = func;
	sys_calls[sys_call_size++] = call;
}

void LoadSysCalls(SysFunc *sys_funcs, char *data, int *header_size)
{
	int i, j;
	int syscall_length = data[(*header_size)++];
	for (i = 0; i < syscall_length; i++)
	{
		int length = data[*header_size];
		char syscall[80];
		memcpy(syscall, data + (*header_size) + 1, length);
		syscall[length] = '\0';
		LOG("System function: '%s'\n", syscall);

		for (j = 0; j < sys_call_size; j++)
		{
			if (!strcmp(syscall, sys_calls[j].name))
			{
				sys_funcs[i] = sys_calls[j].func;
				break;
			}
		}
		*header_size += length + 1;
	}
}

void CleanUp(Object *pointers, int *pointer_count, Object *stack, int sp)
{
	int i, j;
	for (i = 0; i < *pointer_count; i++)
	{
		char has_ref = 0;
		for (j = 0; j < sp; j++)
		{
			if (stack[j].p == pointers[i].p)
			{
				has_ref = 1;
				break;
			}
		}

		if (!has_ref)
		{
			free(pointers[i].p);
			memmove(pointers + i, pointers + i + 1, STACK_SIZE - i);
			LOG("Freed pointer: %x\n", pointers[i].p);
			*pointer_count -= 1;
			i -= 1;
		}
	}
}

void Exec(char *data, int length)
{
	int header_size = 4;
	SysFunc sys_funcs[80];
	LoadSysCalls(sys_funcs, data, &header_size);

	int pc = 0, sp = 0, bp = 0, fp = 0;
	int stack_frame[STACK_SIZE];
	Object stack[STACK_SIZE];

	Object pointers[STACK_SIZE];
	int pointer_count = 0;

	pc = (*((int*)data)) + header_size;
	stack_frame[fp++] = length;
	while (pc < length)
	{
		char bytecode = data[pc++];
		LOG("#%i (%x): ", pc-header_size-1, bytecode);
		switch(bytecode)
		{
			case ALLOC:
				LOG("New stack frame size %i\n", data[pc]);
				stack_frame[fp++] = bp;
				bp = sp;
				sp += data[pc++];
				break;
			
			case PUSH_ARG:
				stack[sp++] = stack[bp-data[pc++]-1]; 
				break;
			
			case PUSH_LOC:
				LOG("Push local #%i\n", data[pc]);
				stack[sp++] = stack[bp+data[pc++]];
				break;
			
			case PUSH_INT:
				LOG("Push int %i\n", *(int*)(data+pc));
				stack[sp++] = (Object){&t_int};
				stack[sp-1].i = *(int*)(data+pc);
				pc += 4;
				break;
			
			case PUSH_FLOAT:
				LOG("Push float %f\n", *(float*)(data+pc));
				stack[sp++] = (Object){&t_float};
				stack[sp-1].f = *(float*)(data+pc);
				pc += 4;
				break;
			
			case PUSH_CHAR:
				LOG("Push char %c(%i)\n", data[pc], data[pc]);
				stack[sp++] = (Object){&t_char};
				stack[sp-1].c = data[pc++];
				break;
			
			case PUSH_BOOL:
				LOG("Push bool %s\n", data[pc] ? "true" : "false");
				stack[sp++] = (Object){&t_bool};
				stack[sp-1].c = data[pc++];
				break;
			
			case PUSH_STRING:
			{
				char *str = (char*)malloc(data[pc]+1);
				memcpy(str, data + pc + 1, data[pc]);
				str[data[pc]] = '\0';
				pc += data[pc] + 1;
				stack[sp++] = (Object){&t_string};
				stack[sp-1].p = (void*)str;
				pointers[pointer_count++] = stack[sp-1];
				LOG("Push string '%s'\n", str);
				break;
			}

			case POP_ARGS:
			{
				LOG("Pop %i arg(s)\n", data[pc]);
				Object temp = stack[--sp];
				sp -= data[pc++];
				stack[sp++] = temp;
				break;
			}

			case POP:
				LOG("Pop %i value(s)\n", data[pc]);
				sp -= data[pc++];
				break;

			case ASSIGN:
				LOG("Assign to local %i\n", data[pc]);
				stack[bp+data[pc++]] = stack[--sp];
				break;

			OP_INSTUCTION(ADD, Add)
			OP_INSTUCTION(SUB, Sub)
			OP_INSTUCTION(MUL, Mul)
			OP_INSTUCTION(DIV, Div)
			OP_INSTUCTION(EQUALS, Equals)
			OP_INSTUCTION(GREATERTHAN, GreaterThan)
			OP_INSTUCTION(LESSTHAN, LessThan)

			case CALL:
				LOG("Call function at %i\n", *((int*)(data + pc)));
				stack_frame[fp++] = pc+4;
				pc = *((int*)(data + pc)) + header_size;
				break;

			case CALL_SYS:
			{
				int index = *((int*)(data + pc));
				pc += 4;

				LOG("Call: %i\n", index);
				SysFunc func = sys_funcs[index];
				func(stack, &sp, pointers, &pointer_count);
				break;
			}
			
			case BRANCH_IF_NOT:
				LOG("Branch if not (%s) by %i\n", stack[sp-1].i ? "false" : "true", data[pc]);
				if (!stack[--sp].i) pc += data[pc]-1;
				pc++;
				break;

			case BRANCH:
				LOG("Branch by %i\n", data[pc]);
				pc += data[pc];
				break;

			case RETURN:
				LOG("Return from function call to %i\n", stack_frame[fp-2]);
				bp = stack_frame[--fp];
				pc = stack_frame[--fp];
				break;

			case INC_LOC:
				LOG("Inc local at %i by %i\n", data[pc], data[pc+1]);
				switch(stack[bp+data[pc]].type->prim)
				{
					case INT: stack[bp+data[pc]].i += data[pc+1]; break;
					case FLOAT: stack[bp+data[pc]].f += data[pc+1]; break;
					case CHAR: stack[bp+data[pc]].c += data[pc+1]; break;
					default: printf("Error: Invalid inc\n"); exit(-1); break;
				}
				pc += 2;
				break;

			case MALLOC:
			{
				LOG("Malloc of size %i\n", data[pc]);
				Object obj;
				obj.type = &t_int;
				obj.p = malloc(sizeof(Object) * data[pc++]);

				int i;
				for (i = 0; i < data[pc-1]; i++)
					((Object*)obj.p)[i] = (Object){&t_int, 0};

				stack[sp++] = obj;
				pointers[pointer_count++] = obj;
				break;
			}

			case PUSH_ATTR:
				LOG("Push attribute at %i\n", data[pc]);
				stack[sp-1] = ((Object*)stack[sp-1].p)[data[pc++]];
				break;
			
			case ASSIGN_ATTR:
				LOG("Assign attribute to %i\n", data[pc]);
				((Object*)stack[sp-1].p)[data[pc++]] = stack[sp-2];
				sp -= 2;
				break;
		}

#if LOG_STACK
		int i;
		LOG("Stack: ");
		for (i = 0; i < sp; i++)
			LOG("%x, ", stack[i].p);
		LOG("\n");
#endif
	}

	sp = 0;
	CleanUp(pointers, &pointer_count, stack, sp);
}

void ExecFile(char *file_path)
{
	FILE *file = fopen(file_path, "rb");
	fseek(file, 0L, SEEK_END);
	int length = ftell(file);
	rewind(file);

	char *data = (char*)malloc(length);
	fread(data, sizeof(char), length, file);
	fclose(file);

	Exec(data, length);
	free(data);
}
