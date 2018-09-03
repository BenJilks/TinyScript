#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include "VM.h"

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

SysFunc sys_funcs[1024];
Type types[80];
Function functions[80];
Pointer *pointers;

int sys_func_size;
int func_size;
int pointer_count;
int cycle;

static Function FindFunc(int id)
{
	return  functions[id];
}
VM vm = {PrimType, AllocPointer, CallFunc, FindFunc};
#include "Operations.h"

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
	CALL_METHOD,
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

void ErrorOut()
{
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

void LoadString(char *str, char *data, int *pointer)
{
	int length = data[(*pointer)];
	memcpy(str, data + (*pointer) + 1, length);
	str[length] = '\0';
	*pointer += length + 1;
}

int LoadInt(char *data, int *pointer)
{
	int i = *(int*)(data + (*pointer));
	*pointer += 4;
	return i;
}

void GetFullPath(char *out, int max_size, char *file)
{
    getcwd(out, max_size);
    int cwd_len = strlen(out);

    out[cwd_len] = '/';
    memcpy(out + cwd_len + 1, file, strlen(file));
    out[cwd_len + strlen(file) + 1] = '\0';
}

void LoadLib(void *handle, char *data, int *pointer)
{
	int i;
	char *error;
	char call[80];
	int call_len = data[(*pointer)++];

	for (i = 0; i < call_len; i++)
	{
		LoadString(call, data, pointer);
		LOG(" ==> Loading external function '%s'\n", call);

		sys_funcs[sys_func_size++] = (SysFunc)dlsym(handle, call);
		if ((error = dlerror()) != NULL)  
		{
			fprintf (stderr, "%s\n", error);
			continue;
		}
	}
}

void LoadExternals(char *data, int *pointer)
{
	int i;
	int len = data[(*pointer)++];
	
	for (i = 0; i < len; i++)
	{
		char file[80];
		char path[1024];
		LoadString(file, data, pointer);
		GetFullPath(path, 1024, file);
		LOG("Loading external '%s'\n", path);

		void *handle = dlopen(path, RTLD_LAZY);
		if (!handle) 
		{
			fprintf(stderr, "%s\n", dlerror());
			return;
		}

		dlerror();
		LoadLib(handle, data, pointer);
	}
}

void LoadDataTypes(char *data, int *pointer)
{
	int i;
	int type_length = data[(*pointer)++];
	for (i = 0; i < type_length; i++)
	{
		Type type;
		LoadString(type.name, data, pointer);
		type.size = LoadInt(data, pointer);
		type.is_sys_type = data[(*pointer)++];
		type.operator_add = LoadInt(data, pointer);
		type.operator_subtract = LoadInt(data, pointer);
		type.operator_multiply = LoadInt(data, pointer);
		type.operator_divide = LoadInt(data, pointer);
		type.operator_to_string = LoadInt(data, pointer);
		type.operator_get_index = LoadInt(data, pointer);
		type.operator_set_index = LoadInt(data, pointer);
		type.operator_it = LoadInt(data, pointer);
		type.prim = OBJECT;

		if (!strcmp(type.name, "String")) { type.prim = STRING; t_string = type; }
		if (!strcmp(type.name, "List")) { type.prim = ARRAY; t_array = type; }
		types[i] = type;
		LOG("Loaded type '%s'\n", type.name);
	}
}

// Load function data from file
void LoadFunctions(char *data, int *pointer)
{
	int i;
	int count = LoadInt(data, pointer);

	for (i = 0; i < count; i++)
	{
		Function func;
		LoadString(func.name, data, pointer);
		func.size = LoadInt(data, pointer);
		func.length = LoadInt(data, pointer);
		func.bytecode = data + (*pointer);
		*pointer += func.length;

		LOG("Loaded function '%s' of size %i\n", func.name, func.length);
		functions[func_size++] = func;
	}
}

// Load and run a program
void LoadProgram(char *program_data, int program_length)
{
	// Init registers and pointer
	pointers = (Pointer*)malloc(sizeof(Pointer) * MEM_SIZE);
	pointer_count = 0;
	sys_func_size = 0;
	func_size = 0;
	cycle = 0;

	// Load program data
	LOG("Loading program\n");
	int pointer = 0;
	LoadExternals(program_data, &pointer);
	LoadDataTypes(program_data, &pointer);
	LoadFunctions(program_data, &pointer);
	LOG("Finished loading program\n");

	// Call main functions
	Function main = functions[LoadInt(program_data, &pointer)];
	CallFunc(main, NULL, 0);

	// Free program data
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
		stack[(--sp)-1] = op(stack[sp-2], stack[sp-1], stack, sp); \
		break; \

#define CHECK_OP(obj, operator) \
	if (obj.type->operator == -1) \
	{ \
		ERROR("Error: No %s of type '%s' found\n", #operator, obj.type->name); \
	}

Object CallFunc(Function func, Object *params, int param_size)
{
	Object stack[80];
	char *data = func.bytecode;
	int sp = func.size, pc = 0;

	int should_return = 0;
	while (pc < func.length && !should_return)
	{
		cycle++;
		char bytecode = func.bytecode[pc++];
		LOG("%s -> #%i (%x): ", func.name, pc, bytecode);
		switch(bytecode)
		{
			case PUSH_ARG:
				LOG("Push argument #%i\n", data[pc]);
				stack[sp++] = params[data[pc++]];
				break;
			
			case PUSH_LOC:
				LOG("Push local #%i\n", data[pc]);
				stack[sp++] = stack[data[pc++]];
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
				stack[data[pc++]] = stack[--sp];
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
			{
				int index = *((int*)(data + pc));
				int arg_size = data[pc+4];
				pc += 5;

				Function func = functions[index];
				LOG("Call function %s(%i), with %i arg(s)\n", func.name, index, arg_size);

				Object ret = CallFunc(func, stack + sp - arg_size, arg_size);
				stack[sp - arg_size] = ret;
				sp -= arg_size - 1;
				break;
			}

			case CALL_SYS:
			{
				int index = *((int*)(data + pc));
				int arg_size = data[pc+4];
				pc += 5;

				LOG("Call external %i with %i arg(s)\n", index, arg_size);
				SysFunc func = sys_funcs[index];
				Object ret = func(stack + sp - arg_size, arg_size, vm);
				stack[sp - arg_size] = ret;
				sp -= arg_size - 1;
				break;
			}

			case CALL_METHOD:
			{
				int index = *((int*)(data + pc));
				int arg_size = data[pc+4];
				pc += 5;

				Function func = functions[index];
				LOG("Call method %s(%i), with %i arg(s)\n", func.name, index, arg_size);

				Object ret = CallFunc(func, stack + sp - arg_size - 1, arg_size);
				stack[sp - arg_size] = ret;
				sp -= arg_size - 1;
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
				LOG("Return from function call\n");
				should_return = 1;
				break;

			case INC_LOC:
				LOG("Inc local at %i by %i\n", data[pc], data[pc+1]);
				switch(stack[data[pc]].type->prim)
				{
					case INT: stack[data[pc]].i += data[pc+1]; break;
					case FLOAT: stack[data[pc]].f += data[pc+1]; break;
					case CHAR: stack[data[pc]].c += data[pc+1]; break;
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
				//CHECK_OP(obj, operator_get_index);
				//CALL_OP(obj, operator_get_index);
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
				stack[(sp-2)] = stack[(sp-3)];
				stack[(sp-3)] = obj;
				//CHECK_OP(obj, operator_set_index);
				//CALL_OP(obj, operator_set_index);
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
					//CHECK_OP(arr, operator_it);
					//CALL_OP(arr, operator_it);
					//arr = stack[--sp];
				}
				//sp--;

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
				stack[data[pc++]] = next;
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
			printf("%s, ", stack[i].type->name);
		printf("\n");
#endif
	}

	return stack[sp - 1];
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
