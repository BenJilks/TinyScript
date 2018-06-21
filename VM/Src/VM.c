#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VM.h"
#include "String.h"
#include "Operations.h"

#define DEBUG 0
#define LOG_STACK 0

#if DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

char *data;
int length;

int header_size;
SysFunc sys_funcs[80];
Type types[80];

int pc, sp, bp, fp;
int stack_frame[STACK_SIZE];
Object stack[STACK_SIZE];

Pointer pointers[STACK_SIZE];
int pointer_count;

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

Pointer *AllocPointer(void *p)
{
	int i;
	for (i = 0; i < pointer_count; i++)
	{
		if (pointers[i].v == NULL)
		{
			pointers[i].v = p;
			return &pointers[i];
		}
	}

	pointers[pointer_count].v = p;
	pointers[pointer_count].ref_count = 0;
	return &pointers[pointer_count++];
}

void LoadString(char *str)
{
	int length = data[header_size];
	memcpy(str, data + header_size + 1, length);
	str[length] = '\0';
	header_size += length + 1;
}

int LoadInt()
{
	int i = *(int*)(data + header_size);
	header_size += 4;
	return i;
}

void LoadSysCalls()
{
	int i, j;
	int syscall_length = data[header_size++];
	for (i = 0; i < syscall_length; i++)
	{
		char syscall[80];
		LoadString(syscall);
		LOG("System function: '%s'\n", syscall);

		for (j = 0; j < sys_call_size; j++)
		{
			if (!strcmp(syscall, sys_calls[j].name))
			{
				sys_funcs[i] = sys_calls[j].func;
				break;
			}
		}
	}
}

void LoadDataTypes()
{
	int i;
	int type_length = data[header_size++];
	for (i = 0; i < type_length; i++)
	{
		Type type;
		LoadString(type.name);
		type.size = LoadInt();
		type.operator_add = LoadInt();
		type.operator_subtract = LoadInt();
		type.operator_multiply = LoadInt();
		type.operator_divide = LoadInt();
		type.operator_to_string = LoadInt();
		type.prim = OBJECT;
		types[i] = type;

		LOG("Loaded type '%s'\n", type.name);
	}
}

void CleanUp()
{
	int i;
	for (i = 0; i < sp; i++)
	{
		Object obj = stack[sp];
		if (obj.type->prim == OBJECT)
			obj.p->ref_count++;
	}

	for (i = 0; i < pointer_count; i++)
	{
		Pointer *p = &pointers[i];
		if (p->ref_count <= 0 && p->v != NULL)
		{
			LOG("Freed pointer 0x%x\n", p->v);
			free(p->v);
			p->v = NULL;
		}
		p->ref_count = 0;
	}
}

void LoadProgram(char *program_data, int program_length)
{
	data = program_data;
	length = program_length;
	pc = 0;
	sp = 0;
	bp = 0;
	fp = 0;
	pointer_count = 0;

	int main_func = *(int*)data;
	header_size = 4;

	LoadSysCalls();
	LoadDataTypes();
	CallFunc(main_func);
	CleanUp();
}

OP_FUNC(Add, +, operator_add, StringAdd);
OP_FUNC(Sub, -, operator_subtract, StringError);
OP_FUNC(Mul, *, operator_multiply, StringMultiply);
OP_FUNC(Div, /, operator_divide, StringError);
COMPARE_FUNC(GreaterThan, >);
COMPARE_FUNC(LessThan, <);

#define OP_INSTUCTION(code, op) \
	case code: \
		LOG("%s operation\n", #op); \
		stack[(--sp)-1] = op(stack[sp-2], stack[sp-1]); \
		break; \

void CallFunc(int func)
{
	int recursion_depth = 1;
	stack_frame[fp++] = pc;
	pc = func + header_size;

	while (recursion_depth > 0 && pc < length)
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
				stack[sp-1].p = AllocPointer(str);
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
				recursion_depth++;
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
				sp = bp + 1;
				bp = stack_frame[--fp];
				pc = stack_frame[--fp];
				recursion_depth--;
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
				LOG("Malloc new '%s' object\n", types[data[pc]].name);
				Object obj;
				obj.type = &types[data[pc++]];
				Object *attrs = (Object*)malloc(sizeof(Object) * obj.type->size);

				int i;
				for (i = 0; i < data[pc-1]; i++)
					attrs[i] = (Object){&t_int, 0};

				obj.p = AllocPointer(attrs);
				stack[sp++] = obj;
				break;
			}

			case PUSH_ATTR:
				LOG("Push attribute at %i\n", data[pc]);
				stack[sp-1] = stack[sp-1].p->attrs[data[pc++]];
				break;
			
			case ASSIGN_ATTR:
				LOG("Assign attribute to %i\n", data[pc]);
				stack[sp-1].p->attrs[data[pc++]] = stack[sp-2];
				sp -= 2;
				break;
		}

#if LOG_STACK
		int i;
		printf("Stack: ");
		for (i = 0; i < sp; i++)
			printf("%x, ", stack[i].type);
		printf("\n");
#endif
	}
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

	LoadProgram(data, length);
	free(data);
}
