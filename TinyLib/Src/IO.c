#include "TinyScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *AsString(Object obj, Object *stack, int *sp, VM vm);

void print(Object *stack, int *sp, VM vm)
{
	Object obj = stack[(*sp)-1];
	char *str = AsString(obj, stack, sp, vm);
	printf("%s", str);
	free(str);

	stack[(*sp)++] = (Object){vm.PrimType(INT), 0};
}

void println(Object *stack, int *sp, VM vm)
{
	print(stack, sp, vm);
	printf("\n");
}

void input(Object *stack, int *sp, VM vm)
{
	printf("%s", stack[(*sp)-1].p->str);

	char *buffer = (char*)malloc(1024);
	size_t size = 1024;
	getline(&buffer, &size, stdin);
	int len = strlen(buffer);
	buffer = (char*)realloc(buffer, len);
	buffer[len-1] = '\0';

	Object obj;
	obj.type = vm.PrimType(STRING);
	obj.p = vm.AllocPointer(buffer);
	stack[(*sp)++] = obj;
}

void as_int(Object *stack, int *sp, VM vm)
{
	Object out;
	Object in = stack[(*sp)-1];

	out.type = vm.PrimType(INT);
	switch(in.type->prim)
	{
		case INT: out.i = in.i; break;
		case FLOAT: out.i = (int)in.f; break;
		case CHAR: out.i = in.c; break;
		case BOOL: out.i = in.c; break;
		case STRING: out.i = atoi(in.p->str); break;
	}
	stack[(*sp)++] = out;
}

void as_float(Object *stack, int *sp, VM vm)
{
	Object out;
	Object in = stack[(*sp)-1];

	out.type = vm.PrimType(FLOAT);
	switch(in.type->prim)
	{
		case INT: out.f = (float)in.i; break;
		case FLOAT: out.f = in.f; break;
		case CHAR: out.f = (float)in.c; break;
		case BOOL: out.f = (float)in.c; break;
		case STRING: out.f = atof(in.p->str); break;
	}
	stack[(*sp)++] = out;
}

int SizeOfObject(Object obj);
int SizeOfArray(Object arr)
{
	int size = 0, i = 0;
	int len = arr.p->attrs[0].i;
	for (i = 0; i < len; i++)
		size += SizeOfObject(arr.p->attrs[i]);
	return size;
}

int SizeOfObjectAttrs(Object obj)
{
	int size = 0, i = 0;
	int len = obj.type->size;
	for (i = 0; i < len; i++)
		size += SizeOfObject(obj.p->attrs[i]);
	return size;
}

int SizeOfObject(Object obj)
{
	int size = sizeof(Object);

	Primitive prim = obj.type->prim;
	switch(prim)
	{
		case OBJECT: size += SizeOfObjectAttrs(obj); break;
		case ARRAY: size += SizeOfArray(obj); break;
		case STRING: size += strlen(obj.p->str); break;
	}
	return size;
}

void size_of(Object *stack, int *sp, VM vm)
{
	Object obj = stack[(*sp)-1];
	stack[(*sp)++] = (Object){vm.PrimType(INT), SizeOfObject(obj)};
}

// Class File

void File_File(Object *stack, int *sp, VM vm)
{
	char *path = (char*)stack[(*sp)-2].p->str;
	char *mode = (char*)stack[(*sp)-1].p->str;
	Object *file_obj = &stack[(*sp)-3];

	FILE *file = fopen(path, mode);
	file_obj->p = vm.AllocPointer(file);
	stack[(*sp)++] = (Object){vm.PrimType(INT), 0};
}

void File_has_error(Object *stack, int *sp, VM vm)
{
	Object *file_obj = &stack[(*sp)-1];
	int has_error = !file_obj->p->v;
	stack[(*sp)++] = (Object){vm.PrimType(BOOL), has_error};
}

void File_read_all(Object *stack, int *sp, VM vm)
{
	Object file_obj = stack[(*sp)-1];
	FILE *file = (FILE*)file_obj.p->v;
	if (!file)
	{
		stack[(*sp)++] = (Object){vm.PrimType(INT), 0};
		return;
	}

	fseek(file, 0L, SEEK_END);
	int length = ftell(file);
	rewind(file);
	char *data = (char*)malloc(length + 1);
	fread(data, sizeof(char), length, file);
	data[length] = '\0';

	Object str_obj;
	str_obj.type = vm.PrimType(STRING);
	str_obj.p = vm.AllocPointer(data);
	stack[(*sp)++] = str_obj;
}

void File_close(Object *stack, int *sp, VM vm)
{
	Object file_obj = stack[(*sp)-1];
	FILE *file = (FILE*)file_obj.p->v;
	int error = 0;
	if (file)
		error = fclose(file);
	stack[(*sp)++] = (Object){vm.PrimType(INT), error};
}

void File_operator_it(Object *stack, int *sp, VM vm)
{
	Object file_obj = stack[(*sp)-1];
	FILE *file = (FILE*)file_obj.p->v;
	Object *attrs = (Object*)malloc(sizeof(Object) * 100);
	int counter = 1;

	if (file)
	{
		char * line = NULL;
		size_t len = 0;
		size_t read;
		while ((read = getline(&line, &len, file)) != -1) 
		{
			int size = strlen(line);
			char *line_str = (char*)malloc(size);
			memcpy(line_str, line, size-1);
			line_str[size-1] = '\0';

			Object item;
			item.type = vm.PrimType(STRING);
			item.p = vm.AllocPointer(line_str);
			attrs[counter++] = item;
		}
	}
	attrs[0] = (Object){vm.PrimType(INT), counter-1};
	attrs = (Object*)realloc(attrs, sizeof(Object) * counter);

	Object obj;
	obj.type = vm.PrimType(ARRAY);
	obj.p = vm.AllocPointer(attrs);
	stack[(*sp)++] = obj;
}
