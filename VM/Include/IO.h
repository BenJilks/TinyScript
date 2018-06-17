#include "VM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void AsString(Object obj, char *str, Object *stack, int *sp)
{
	switch(obj.type->prim)
	{
		case INT: sprintf(str, "%i", obj.i); break;
		case FLOAT: sprintf(str, "%.6g", obj.f); break;
		case CHAR: sprintf(str, "%c", obj.c); break;
		case BOOL: strcpy(str, obj.c ? "true" : "false"); break;
		case STRING: strcpy(str, (char*)obj.p); break;
		case OBJECT:
			if (obj.type->operator_to_string != -1)
			{
				stack[(*sp)++] = obj;
				CallFunc(obj.type->operator_to_string);
				strcpy(str, (char*)stack[(*sp)-1].p);
				*sp -= 2;
				break;
			}
			sprintf(str, "<%s at 0x%x>", obj.type->name, obj.p); 
			break;
	}
}

void Print(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	Object obj = stack[(*sp)-1];
	char str[80];
	AsString(obj, str, stack, sp);
	printf("%s", str);

	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void Println(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	Print(stack, sp, pointers, pointer_count);
	printf("\n");
}

void Input(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	printf("%s", (char*)stack[(*sp)-1].p);

	char *buffer = (char*)malloc(1024);
	scanf("%[^\n]s", buffer);
	buffer = (char*)realloc(buffer, strlen(buffer) + 1);

	Object obj;
	obj.type = PrimType(STRING);
	obj.p = (void*)buffer;
	stack[(*sp)++] = obj;
	pointers[(*pointer_count)++] = obj;
}

void Int(Object *stack, int *sp, Object *pointers, int *pointer_count)
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
		case STRING: out.i = atoi((char*)in.p); break;
	}
	stack[(*sp)++] = out;
}

void Float(Object *stack, int *sp, Object *pointers, int *pointer_count)
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
		case STRING: out.f = atof((char*)in.p); break;
	}
	stack[(*sp)++] = out;
}

void String(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	Object out;
	Object in = stack[(*sp)-1];

	out.type = PrimType(STRING);
	out.p = malloc(1024);
	AsString(in, (char*)out.p, stack, sp);
	out.p = realloc(out.p, strlen((char*)out.p) + 1);
	stack[(*sp)++] = out;
	pointers[(*pointer_count)++] = out;
}

// Class String

void String_Length(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	Object str = stack[(*sp)-1];
	int length = strlen((char*)str.p);

	Object size_obj;
	size_obj.type = PrimType(INT);
	size_obj.i = length;
	stack[(*sp)++] = size_obj;
}

void String_At(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	char *str = (char*)stack[(*sp)-2].p;
	int index = stack[(*sp)-1].i;

	Object c_obj;
	c_obj.type = PrimType(CHAR);
	c_obj.c = str[index];
	stack[(*sp)++] = c_obj;
}

void String_Append(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	char *dest = (char*)stack[(*sp)-2].p;
	char src[80];
	AsString(stack[(*sp)-1], src, stack, sp);

	int dest_len = strlen(dest);
	int src_len = strlen(src);
	dest = (char*)realloc(dest, dest_len + src_len + 1);
	memcpy(dest + dest_len, src, src_len);
	dest[dest_len + src_len] = '\0';

	stack[(*sp)-2].p = (void*)dest;
	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

// Class File

void File_File(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	char *path = (char*)stack[(*sp)-2].p;
	char *mode = (char*)stack[(*sp)-1].p;
	Object *file_obj = &stack[(*sp)-3];

	FILE *file = fopen(path, mode);
	file_obj->p = realloc(file_obj->p, sizeof(FILE*));
	memcpy(file_obj->p, &file, sizeof(FILE*));
	stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void File_ReadAll(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	Object file_obj = stack[(*sp)-1];
	FILE *file = *(FILE**)file_obj.p;

	fseek(file, 0L, SEEK_END);
	int length = ftell(file);
	rewind(file);
	char *data = (char*)malloc(length + 1);
	fread(data, sizeof(char), length, file);
	data[length] = '\0';

	Object str_obj;
	str_obj.type = PrimType(STRING);
	str_obj.p = (void*)data;
	stack[(*sp)++] = str_obj;
	pointers[(*pointer_count)++] = str_obj;
}

void File_Close(Object *stack, int *sp, Object *pointers, int *pointer_count)
{
	Object file_obj = stack[(*sp)-1];
	int error = fclose(*(FILE**)file_obj.p);
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

