#ifndef IO_H
#define IO_H

#include "VM.h"
#include "String.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Print(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	Object obj = stack[(*sp)-1];
	char *str = (char*)malloc(1024);
	AsString(obj, str, stack, sp);
	printf("%s", str);
	free(str);

	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void Println(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	Print(stack, sp, pointers, pointer_count);
	printf("\n");
}

void Input(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
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

void Int(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
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

void Float(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
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

void String(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	Object in = stack[(*sp)-1];
	char *str = (char*)malloc(1024);
	AsString(in, str, stack, sp);
	str = (char*)realloc(str, strlen(str) + 1);

	Object out;
	out.type = PrimType(STRING);
	out.p = AllocPointer(str);
	stack[(*sp)++] = out;
}

// Class String

void String_Length(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	Object str = stack[(*sp)-1];
	int length = strlen(str.p->str);

	Object size_obj;
	size_obj.type = PrimType(INT);
	size_obj.i = length;
	stack[(*sp)++] = size_obj;
}

void String_At(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	char *str = stack[(*sp)-2].p->str;
	int index = stack[(*sp)-1].i;

	Object c_obj;
	c_obj.type = PrimType(CHAR);
	c_obj.c = str[index];
	stack[(*sp)++] = c_obj;
}

void String_Append(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	char *dest = stack[(*sp)-2].p->str;
	char src[80];
	AsString(stack[(*sp)-1], src, stack, sp);

	int dest_len = strlen(dest);
	int src_len = strlen(src);
	dest = (char*)realloc(dest, dest_len + src_len + 1);
	memcpy(dest + dest_len, src, src_len);
	dest[dest_len + src_len] = '\0';

	stack[(*sp)-2].p->str = dest;
	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

// Class File

void File_File(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	char *path = (char*)stack[(*sp)-2].p->str;
	char *mode = (char*)stack[(*sp)-1].p->str;
	Object file_obj = stack[(*sp)-3];
	free(file_obj.p->v);

	FILE *file = fopen(path, mode);
	file_obj.p->v = file;
	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void File_ReadAll(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
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

void File_Close(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
	Object file_obj = stack[(*sp)-1];
	int error = fclose((FILE*)file_obj.p->v);
	stack[(*sp)++] = (Object){PrimType(INT), error};
}

void RegisterIO()
{
	RegisterFunc((char*)"Print", Print);
	RegisterFunc((char*)"Println", Println);
	RegisterFunc((char*)"Input", Input);
	RegisterFunc((char*)"Int", Int);
	RegisterFunc((char*)"Float", Float);
	RegisterFunc((char*)"AsString", String);

	RegisterFunc((char*)"String:Length", String_Length);
	RegisterFunc((char*)"String:At", String_At);
	RegisterFunc((char*)"String:Append", String_Append);

	RegisterFunc((char*)"File:File", File_File);
	RegisterFunc((char*)"File:ReadAll", File_ReadAll);
	RegisterFunc((char*)"File:Close", File_Close);
}

#endif // IO_H
