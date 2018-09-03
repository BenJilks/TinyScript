#include "TinyScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *AsString(Object obj, VM vm);

Object print(Object *args, int arg_size, VM vm)
{
	int i;
	for (i = 0; i < arg_size; i++)
	{
		Object obj = args[i];
		char *str = AsString(obj, vm);
		printf("%s", str);
		free(str);
	}

	return (Object){vm.PrimType(INT), 0};
}

Object println(Object *args, int arg_size, VM vm)
{
	Object obj = print(args, arg_size, vm);
	printf("\n");
	return obj;
}

Object call_func(Object *args, int arg_size, VM vm)
{
	int func_id = args[0].i;
	Function func = vm.FindFunc(func_id);
	return vm.CallFunc(func, args + 1, arg_size - 1);
}

Object input(Object *args, int arg_size, VM vm)
{
	printf("%s", args[0].p->str);

	char *buffer = (char*)malloc(1024);
	size_t size = 1024;
	getline(&buffer, &size, stdin);
	int len = strlen(buffer);
	buffer = (char*)realloc(buffer, len);
	buffer[len-1] = '\0';

	Object obj;
	obj.type = vm.PrimType(STRING);
	obj.p = vm.AllocPointer(buffer);
	return obj;
}

Object as_int(Object *args, int arg_size, VM vm)
{
	Object out;
	Object in = args[0];

	out.type = vm.PrimType(INT);
	switch(in.type->prim)
	{
		case INT: out.i = in.i; break;
		case FLOAT: out.i = (int)in.f; break;
		case CHAR: out.i = in.c; break;
		case BOOL: out.i = in.c; break;
		case STRING: out.i = atoi(in.p->str); break;
	}
	return out;
}

Object as_float(Object *args, int arg_size, VM vm)
{
	Object out;
	Object in = args[0];

	out.type = vm.PrimType(FLOAT);
	switch(in.type->prim)
	{
		case INT: out.f = (float)in.i; break;
		case FLOAT: out.f = in.f; break;
		case CHAR: out.f = (float)in.c; break;
		case BOOL: out.f = (float)in.c; break;
		case STRING: out.f = atof(in.p->str); break;
	}
	return out;
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

Object size_of(Object *args, int arg_size, VM vm)
{
	Object obj = args[0];
	return (Object){vm.PrimType(INT), SizeOfObject(obj)};
}

// Class File

Object File_File(Object *args, int arg_size, VM vm)
{
	char *path = (char*)args[0].p->str;
	char *mode = (char*)args[1].p->str;
	Object *file_obj = &args[2];

	FILE *file = fopen(path, mode);
	file_obj->p = vm.AllocPointer(file);
	return (Object){vm.PrimType(INT), 0};
}

Object File_has_error(Object *args, int arg_size, VM vm)
{
	Object *file_obj = &args[0];
	int has_error = !file_obj->p->v;
	return (Object){vm.PrimType(BOOL), has_error};
}

Object File_read_all(Object *args, int arg_size, VM vm)
{
	Object file_obj = args[0];
	FILE *file = (FILE*)file_obj.p->v;
	if (!file)
		return (Object){vm.PrimType(INT), 0};

	fseek(file, 0L, SEEK_END);
	int length = ftell(file);
	rewind(file);
	char *data = (char*)malloc(length + 1);
	fread(data, sizeof(char), length, file);
	data[length] = '\0';

	Object str_obj;
	str_obj.type = vm.PrimType(STRING);
	str_obj.p = vm.AllocPointer(data);
	return str_obj;
}

Object File_close(Object *args, int arg_size, VM vm)
{
	Object file_obj = args[0];
	FILE *file = (FILE*)file_obj.p->v;
	int error = 0;
	if (file)
		error = fclose(file);
	return (Object){vm.PrimType(INT), error};
}

Object File_operator_it(Object *args, int arg_size, VM vm)
{
	Object file_obj = args[0];
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
	return obj;
}
