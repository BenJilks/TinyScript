#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VM.h"
#include "String.h"
#include "Operations.h"

#define DEBUG 0
#define DEBUG_MEM 0
#define LOG_STACK 0
#define MEM_CLEAR_RATE 500

#if DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#if DEBUG_MEM
#define LOG_MEM(...) printf(__VA_ARGS__)
#else
#define LOG_MEM(...)
#endif

char *data;
int length;

int header_size;
SysFunc sys_funcs[80];
Type types[80];

int pc, sp, bp, fp, cycle;
int stack_frame[STACK_SIZE];
Object stack[STACK_SIZE];

Pointer *pointers;
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
	GREATERTHANEQUAL,
	LESSTHAN,
	LESSTHANEQUAL,
	AND,
	OR,
	CALL,
	CALL_SYS,
	RETURN,
	BRANCH_IF_NOT,
	BRANCH_IF_IT,
	BRANCH,
	INC_LOC,
	POP_ARGS,
	MALLOC,
	PUSH_ATTR,
	PUSH_INDEX,
	ASSIGN_ATTR,
	ASSIGN_INDEX,
	MAKE_ARRAY,
	MAKE_IT,
	IT_NEXT
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
		case ARRAY: return &t_array;
		case BOOL: return &t_bool;
		case CHAR: return &t_char;
	}
	return NULL;
}

// Scan an object for refrences to a pointer
void ScanObject(Object obj)
{
	int i;
	if (obj.type->prim == STRING)
	{
		obj.p->ref_count++;
	}
	else if (obj.type->prim == ARRAY)
	{
		obj.p->ref_count++;
		for (i = 1; i < obj.p->attrs[0].i; i++)
			ScanObject(obj.p->attrs[i]);
	}
	else if (obj.type->prim == OBJECT)
	{
		obj.p->ref_count++;
		for (i = 0; i < obj.type->size; i++)
			ScanObject(obj.p->attrs[i]);
	}
}

void CleanUp()
{
	int i;
	for (i = 0; i < sp; i++)
		if (stack[i].type != NULL)
			ScanObject(stack[i]);

	for (i = 0; i < pointer_count; i++)
	{
		Pointer *p = &pointers[i];
		if (p->ref_count <= 0 && p->v != NULL)
		{
			LOG_MEM("Freed pointer 0x%x (at %i)\n", p->v, i);
			free(p->v);
			p->v = NULL;
		}
		p->ref_count = 0;
	}
}

void ErrorOut()
{
	sp = 0;
	CleanUp();
	free(pointers);
	exit(0);
}

void CheckMemory()
{
	if (pointer_count >= MEM_SIZE - 1)
	{
		printf("Error: Out of memory\n");
		ErrorOut();
	}
	
	if (cycle > MEM_CLEAR_RATE)
	{
		CleanUp();
		cycle = 0;
	}
}

Pointer *AllocPointer(void *p)
{
	CheckMemory();
	int i;
	for (i = 0; i < pointer_count; i++)
	{
		if (pointers[i].v == NULL)
		{
			LOG_MEM("New pointer 0x%x (at %i, override)\n", p, i);
			pointers[i].v = p;
			return &pointers[i];
		}
	}

	LOG_MEM("New pointer 0x%x (at %i)\n", p, pointer_count);
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

		int was_found = 0;
		for (j = 0; j < sys_call_size; j++)
		{
			if (!strcmp(syscall, sys_calls[j].name))
			{
				sys_funcs[i] = sys_calls[j].func;
				was_found = 1;
				break;
			}
		}
		LOG("System function: '%s': %s\n", syscall, was_found ? "Found" : "Not Found");

		if (!was_found)
		{
			printf("Error: No system function named '%s' could be found\n", syscall);
			ErrorOut();
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
		type.is_sys_type = data[header_size++];
		type.operator_add = LoadInt();
		type.operator_subtract = LoadInt();
		type.operator_multiply = LoadInt();
		type.operator_divide = LoadInt();
		type.operator_to_string = LoadInt();
		type.operator_get_index = LoadInt();
		type.operator_set_index = LoadInt();
		type.operator_it = LoadInt();
		type.prim = OBJECT;

		if (!strcmp(type.name, "String")) { type.prim = STRING; t_string = type; }
		if (!strcmp(type.name, "List")) { type.prim = ARRAY; t_array = type; }
		types[i] = type;
		LOG("Loaded type '%s'\n", type.name);
	}
}

void ResetReg()
{
	pc = 0;
	sp = 0;
	bp = 0;
	fp = 0;
	cycle = 0;
	pointer_count = 0;
}

void LoadProgram(char *program_data, int program_length)
{
	pointers = (Pointer*)malloc(sizeof(Pointer) * MEM_SIZE);
	data = program_data;
	length = program_length;

	int main_func = *(int*)data;
	header_size = 4;

	LOG("Linking program...\n");
	ResetReg();
	LoadSysCalls();
	LoadDataTypes();
	LOG("Finished linking\n");

	CallFunc(main_func);
	//CleanUp();
	free(pointers);
}

OP_FUNC(Add, +, operator_add);
OP_FUNC(Sub, -, operator_subtract);
OP_FUNC(Mul, *, operator_multiply);
OP_FUNC(Div, /, operator_divide);
COMPARE_FUNC(GreaterThan, >);
COMPARE_FUNC(GreaterThanEqual, >=);
COMPARE_FUNC(LessThan, <);
COMPARE_FUNC(LessThanEqual, <=);
LOGIC_FUNC(And, &&);
LOGIC_FUNC(Or, ||);

#define OP_INSTUCTION(code, op) \
	case code: \
		LOG("%s operation\n", #op); \
		stack[(--sp)-1] = op(stack[sp-2], stack[sp-1]); \
		break; \

#define CHECK_OP(obj, operator) \
	if (obj.type->operator == -1) \
	{ \
		ERROR("Error: No %s of type '%s' found\n", #operator, obj.type->name); \
	}

void log_object(Object obj)
{
	printf("%s", obj.type->name);
	if (obj.type != NULL)
	{
		switch (obj.type->prim)
		{
			case INT: printf("(%i)", obj.i); break;
			case OBJECT: 
			{
				printf("(");
				int j;
				for (j = 0; j < obj.type->size; j++)
					log_object(obj.p->attrs[j]);
				printf(")");
				break;
			}
		}
	}
	printf(", ");
}

void CallFunc(int func)
{
	int recursion_depth = 1;
	stack_frame[fp++] = pc;
	pc = func + header_size;

	while (recursion_depth > 0 && pc < length)
	{
		cycle++;
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
				int length = data[pc];
				char *str = (char*)malloc(length+1);
				memcpy(str, data + pc + 1, length);
				str[length] = '\0';
				pc += length + 1;

				Object obj;
				obj.type = &t_string;
				obj.p = AllocPointer(str);
				stack[sp++] = obj;
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
			OP_INSTUCTION(GREATERTHANEQUAL, GreaterThanEqual)
			OP_INSTUCTION(LESSTHAN, LessThan)
			OP_INSTUCTION(LESSTHANEQUAL, LessThanEqual)
			OP_INSTUCTION(AND, And)
			OP_INSTUCTION(OR, Or)

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
				func(stack, &sp);
				break;
			}
			
			case BRANCH_IF_NOT:
				LOG("Branch if not (%s) by %i\n", stack[sp-1].i ? "false" : "true", *((int*)(data + pc)));
				if (!stack[--sp].i) pc += (*((int*)(data + pc)))-4;
				pc += 4;
				break;

			case BRANCH:
				LOG("Branch by %i\n", *((int*)(data + pc)));
				pc += *((int*)(data + pc));
				break;

			case RETURN:
				LOG("Return from function call to %i\n", stack_frame[fp-2]);
				stack[bp] = stack[sp-1];
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
					default: printf("Error: Invalid inc\n"); ErrorOut(); break;
				}
				pc += 2;
				break;

			case MALLOC:
			{
				LOG("Malloc new %i[%s] object (of size %i bytes)\n", data[pc], types[data[pc]].name, sizeof(Object) * types[data[pc]].size);
				Object obj;
				obj.type = &types[data[pc++]];
				if (obj.type->size > 0)
				{
					Object *attrs = (Object*)calloc(obj.type->size, sizeof(Object));
					obj.p = AllocPointer(attrs);
				}
				stack[sp++] = obj;
				break;
			}

			case PUSH_ATTR:
				LOG("Push attribute at %i\n", data[pc]);
				stack[sp-1] = stack[sp-1].p->attrs[data[pc++]];
				break;

			case PUSH_INDEX:
			{
				LOG("Push index\n", data[pc]);
				Object obj = stack[(sp-2)];
				CHECK_OP(obj, operator_get_index);
				CALL_OP(obj, operator_get_index);
				stack[sp-3] = stack[sp-1];
				sp -= 2;
				break;
			}
			
			case ASSIGN_ATTR:
				LOG("Assign attribute to %i\n", data[pc]);
				stack[sp-1].p->attrs[data[pc++]] = stack[sp-2];
				sp -= 2;
				break;
			
			case ASSIGN_INDEX:
			{
				LOG("Assign index\n", data[pc]);
				Object obj = stack[(sp-2)];
				CHECK_OP(obj, operator_set_index);
				CALL_OP(obj, operator_set_index);
				sp -= 4;
				break;
			}

			case MAKE_ARRAY:
			{
				LOG("Make array of size %i\n", data[pc]);
				Object *attrs = (Object*)malloc(sizeof(Object) * (data[pc]+1));
				attrs[0] = (Object){&t_int, data[pc]};
				memcpy(attrs + 1, stack + sp - data[pc], data[pc] * sizeof(Object));
				sp -= data[pc++];

				Object obj = (Object){&t_array};
				obj.p = AllocPointer(attrs);
				stack[sp++] = obj;
				break;
			}

			case MAKE_IT:
			{
				LOG("Make a new iterator\n");
				
				// Fetch the array object
				Object arr = stack[sp-1];
				if (arr.type->prim == OBJECT)
				{
					CHECK_OP(arr, operator_it);
					CALL_OP(arr, operator_it);
					arr = stack[--sp];
				}
				sp--;

				// Create an object with a counter and array
				Object *attrs = (Object*)malloc(sizeof(Object) * 2);
				attrs[0] = (Object){&t_int, 0};
				attrs[1] = arr;

				// Push new object to stack
				Object it;
				it.type = &t_array;
				it.p = AllocPointer(attrs);
				stack[sp++] = it;
				break;
			}

			case IT_NEXT:
			{
				LOG("Next in iterator and store in local %i\n", data[pc]);
				Object it = stack[sp-1];

				int index = it.p->attrs[0].i++;
				Object next = it.p->attrs[1].p->attrs[index+1];
				stack[bp+data[pc++]] = next;
				break;
			}

			case BRANCH_IF_IT:
			{
				LOG("Branch if iterator does not have a next value by %i\n", *((int*)(data + pc)));
				Object it = stack[sp-1];
				int index = it.p->attrs[0].i;
				int size = it.p->attrs[1].p->attrs[0].i;

				if (index >= size)
					pc += (*((int*)(data + pc)))-4;
				pc += 4;
				break;
			}
		}

#if LOG_STACK
		int i;
		printf("Stack: ");
		for (i = 0; i < sp; i++)
			log_object(stack[i]);
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
