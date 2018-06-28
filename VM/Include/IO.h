#ifndef IO_H
#define IO_H

#include "VM.h"
#include "String.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Print(Object *stack, int *sp)
{
	Object obj = stack[(*sp)-1];
	char *str = AsString(obj, stack, sp);
	printf("%s", str);
	free(str);

	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void Println(Object *stack, int *sp)
{
	Print(stack, sp);
	printf("\n");
}

void Input(Object *stack, int *sp)
{
	printf("%s", stack[(*sp)-1].p->str);

	char *buffer = (char*)malloc(1024);
	size_t size = 1024;
	getline(&buffer, &size, stdin);
	buffer = (char*)realloc(buffer, size + 1);
	buffer[size] = '\0';

	Object obj;
	obj.type = PrimType(STRING);
	obj.p = AllocPointer(buffer);
	stack[(*sp)++] = obj;
}

void Int(Object *stack, int *sp)
{
	Object out;
	Object in = stack[(*sp)-1];

	out.type = PrimType(INT);
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

void Float(Object *stack, int *sp)
{
	Object out;
	Object in = stack[(*sp)-1];

	out.type = PrimType(FLOAT);
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

// Class File

void File_File(Object *stack, int *sp)
{
	char *path = (char*)stack[(*sp)-2].p->str;
	char *mode = (char*)stack[(*sp)-1].p->str;
	Object file_obj = stack[(*sp)-3];
	free(file_obj.p->v);

	FILE *file = fopen(path, mode);
	file_obj.p->v = file;
	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void File_ReadAll(Object *stack, int *sp)
{
	Object file_obj = stack[(*sp)-1];
	FILE *file = (FILE*)file_obj.p->v;

	fseek(file, 0L, SEEK_END);
	int length = ftell(file);
	rewind(file);
	char *data = (char*)malloc(length + 1);
	fread(data, sizeof(char), length, file);
	data[length] = '\0';

	Object str_obj;
	str_obj.type = PrimType(STRING);
	str_obj.p = AllocPointer(data);
	stack[(*sp)++] = str_obj;
}

void File_Close(Object *stack, int *sp)
{
	Object file_obj = stack[(*sp)-1];
	int error = fclose((FILE*)file_obj.p->v);
	stack[(*sp)++] = (Object){PrimType(INT), error};
}

void RegisterIO()
{
	RegisterFunc((char*)"print", Print);
	RegisterFunc((char*)"println", Println);
	RegisterFunc((char*)"input", Input);
	RegisterFunc((char*)"int", Int);
	RegisterFunc((char*)"float", Float);

	RegisterFunc((char*)"File:File", File_File);
	RegisterFunc((char*)"File:read_all", File_ReadAll);
	RegisterFunc((char*)"File:close", File_Close);
}

#endif // IO_H
