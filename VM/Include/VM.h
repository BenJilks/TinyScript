#ifndef VM_H
#define VM_H

#define MAX_NAME_LENGTH 80
#define STACK_SIZE 80

typedef enum Primitive
{
	INT,
	FLOAT,
	STRING,
	BOOL,
	CHAR,
	OBJECT
} Primitive;

typedef struct Type
{
	char name[MAX_NAME_LENGTH];
	Primitive prim;
	int size;

	int operator_add;
	int operator_subtract;
	int operator_multiply;
	int operator_divide;
	int operator_to_string;
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

typedef void (*SysFunc)(Object *stack, int *sp, Pointer *pointers, int *pointer_count);
void RegisterFunc(char *name, SysFunc func);
Pointer *AllocPointer(void *p);
void LoadProgram(char *program_data, int program_length);
void CallFunc(int func);
void ExecFile(char *file_path);
Type *PrimType(int prim);

#endif // VM_H
