#include "TinyScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *AsString(Object obj, Object *stack, int *sp, VM vm);
char *ArrayAsString(Object obj, char *str, Object *stack, int *sp, VM vm)
{
    int size = obj.p->attrs[0].i;
    int i = 0, curr_p = 1;
    str[0] = '[';
    for (i = 0; i < size; i++)
    {
        Object attr = obj.p->attrs[i+1];
        char *item = AsString(attr, stack, sp, vm);
        int item_len = strlen(item);

        str = (char*)realloc(str, curr_p + item_len + 4);
        strcpy(str + curr_p, item);
        if (i < size - 1)
            strcpy(str + curr_p + item_len, ", ");

        curr_p = strlen(str);
        free(item);
    }
    str[curr_p] = ']';
    str[curr_p + 1] = '\0';
    return str;
}

char *ObjectAsString(Object obj, char *str, Object *stack, int *sp, VM vm)
{
    if (obj.type->operator_to_string != -1)
    {
        stack[(*sp)++] = obj;
        vm.CallFunc(obj.type->operator_to_string);
		Object ret = stack[(*sp)-1];
	
        str = (char*)realloc(str, strlen(ret.p->str)+1);
        strcpy(str, ret.p->str);
        *sp -= 2;
        return str;
    }
    sprintf(str, "<%s at 0x%x>", obj.type->name, obj.p->v); 
    return str;
}

char *AsString(Object obj, Object *stack, int *sp, VM vm)
{
	char *str = (char*)malloc(80);
	if (obj.type == NULL)
	{
		strcpy(str, "null");
		return str;
	}

	switch(obj.type->prim)
	{
		case INT: sprintf(str, "%i", obj.i); break;
		case FLOAT: sprintf(str, "%.6g", obj.f); break;
		case CHAR: sprintf(str, "%c", obj.c); break;
		case BOOL: strcpy(str, obj.c ? "true" : "false"); break;
		case STRING:
			str = (char*)realloc(str, strlen(obj.p->str)+1);
			strcpy(str, obj.p->str);
		       	break;
		case ARRAY: str = ArrayAsString(obj, str, stack, sp, vm); break;
		case OBJECT: str = ObjectAsString(obj, str, stack, sp, vm); break;
	}
	return str;
}

void String_operator_add(Object *stack, int *sp, VM vm)
{
	// Fetch operation data
    Object left = stack[(*sp)-2];
    Object right = stack[(*sp)-1];
    char *right_str = AsString(right, stack, sp, vm);

	// Create a new combined string
    int left_len = strlen(left.p->str);
    int right_len = strlen(right_str);
    char *str = (char*)malloc(left_len + right_len + 1);
    strcpy(str, left.p->str);
	strcpy(str + left_len, right_str);
	free(right_str);
	
	// Return the new string
	Object result;
	result.type = vm.PrimType(STRING);
	result.p = vm.AllocPointer(str);
	stack[(*sp)++] = result;
}

void String_operator_multiply(Object *stack, int *sp, VM vm)
{
	// Fetch operation data
	Object left = stack[(*sp)-2];
    Object right = stack[(*sp)-1];
	if (right.type != vm.PrimType(INT))
	{
		printf("Error: cannot multiply a string by a '%s'\n", 
			right.type->name);
		return;
	}

	// Create a new string, and fill
	int i;
	int left_len = strlen(left.p->str);
	char *str = (char*)malloc(left_len * right.i + 1);
	for (i = 0; i < right.i; i++)
		strcpy(str + (i * left_len), left.p->str);
	str[left_len * right.i] = '\0';
	
	// Return the new string
	Object result;
	result.type = vm.PrimType(STRING);
	result.p = vm.AllocPointer(str);
	stack[(*sp)++] = result;
}

void StringError(Object *stack, int *sp)
{
	printf("Error: invalid string operation\n");
}

void str(Object *stack, int *sp, VM vm)
{
	Object in = stack[(*sp)-1];
	char *str = AsString(in, stack, sp, vm);

	Object out;
	out.type = vm.PrimType(STRING);
	out.p = vm.AllocPointer(str);
	stack[(*sp)++] = out;
}

void String_size(Object *stack, int *sp, VM vm)
{
	Object str = stack[(*sp)-1];
	int length = strlen(str.p->str);

	Object size_obj;
	size_obj.type = vm.PrimType(INT);
	size_obj.i = length;
	stack[(*sp)++] = size_obj;
}

void String_append(Object *stack, int *sp, VM vm)
{
	char *dest = stack[(*sp)-2].p->str;
	char *src = AsString(stack[(*sp)-1], stack, sp, vm);

	int dest_len = strlen(dest);
	int src_len = strlen(src);
	dest = (char*)realloc(dest, dest_len + src_len + 1);
	memcpy(dest + dest_len, src, src_len);
	dest[dest_len + src_len] = '\0';
	free(src);

	stack[(*sp)-2].p->str = dest;
	stack[(*sp)++] = (Object){vm.PrimType(INT), 0};
}

void String_split(Object *stack, int *sp, VM vm)
{
	Object str_obj = stack[(*sp) - 2];
	Object c_obj = stack[(*sp) - 1];
	char *str = str_obj.p->str;
	char sc = c_obj.c;
	int str_len = strlen(str);

	Object *attr = (Object*)malloc(sizeof(Object) * 100);
	int i = 0, bp = 0, count = 1;
	char buffer[1024];
	for (i = 0; i < str_len; i++)
	{
		char c = str[i];
		if (c == sc || i == str_len-1)
		{
			if (i == str_len-1) buffer[bp++] = c;
			char *item = (char*)malloc(bp + 1);
			memcpy(item, buffer, bp);
			item[bp] = '\0';

			attr[count].type = vm.PrimType(STRING);
			attr[count].p = vm.AllocPointer(item);
			count++;
			bp = 0;
			continue;
		}
		buffer[bp++] = c;
	}
	attr[0] = (Object){vm.PrimType(INT), count-1};
	attr = (Object*)realloc(attr, sizeof(Object) * count);

	Object out;
	out.type = vm.PrimType(ARRAY);
	out.p = vm.AllocPointer(attr);
	stack[(*sp)++] = out;
}
