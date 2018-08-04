#ifndef TINY_SCRIPT_H
#define TINY_SCRIPT_H

#define MAX_NAME_LENGTH 80
#define STACK_SIZE 80
#define MEM_SIZE 1024 * 1024

typedef enum Primitive
{
	INT,
	FLOAT,
	STRING,
	ARRAY,
	BOOL,
	CHAR,
	OBJECT
} Primitive;

typedef struct Type
{
	char name[MAX_NAME_LENGTH];
	Primitive prim;
	int size;
	char is_sys_type;

	int operator_add;
	int operator_subtract;
	int operator_multiply;
	int operator_divide;
	int operator_to_string;
	int operator_get_index;
	int operator_set_index;
	int operator_it;
} Type;

struct Object;
typedef struct Pointer
{
	int ref_count;
	union
	{
		void *v;
		char *str;
		struct Object *attrs;
	};
} Pointer;

typedef struct Object
{
	Type *type;
	union
	{
		int i;
		float f;
		char c;
		Pointer *p;
	};
} Object;

typedef Type *(*PrimTypeFunc)(int);
typedef Pointer *(*AllocPointerFunc)(void *p);
typedef void (*CallFuncFunc)(int);
typedef struct VM
{
	PrimTypeFunc PrimType;
	AllocPointerFunc AllocPointer;
	CallFuncFunc CallFunc;
} VM;
typedef void (*SysFunc)(Object *stack, int *sp, VM vm);

#endif // TINY_SCRIPT_H
