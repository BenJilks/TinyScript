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
} Type;


typedef struct Object
{
	Type *type;
	union
	{
		int i;
		float f;
		char c;
		void *p;
	};
} Object;

typedef void (*SysFunc)(Object *stack, int *sp, Object *pointers, int *pointer_count);
void RegisterFunc(char *name, SysFunc func);
void Exec(char *data, int length);
void ExecFile(char *file_path);
Type *PrimType(int prim);

#endif // VM_H
