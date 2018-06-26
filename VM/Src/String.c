#include "String.h"

void AsString(Object obj, char *str, Object *stack, int *sp)
{
	switch(obj.type->prim)
	{
		case INT: sprintf(str, "%i", obj.i); break;
		case FLOAT: sprintf(str, "%.6g", obj.f); break;
		case CHAR: sprintf(str, "%c", obj.c); break;
		case BOOL: strcpy(str, obj.c ? "true" : "false"); break;
		case STRING: strcpy(str, obj.p->str); break;
		case ARRAY:
		{
			int size = obj.p->attrs[0].i;
			int i = 0, curr_p = 1;
			str[0] = '[';
			for (i = 0; i < size; i++)
			{
				Object attr = obj.p->attrs[i+1];
				AsString(attr, str + curr_p, stack, sp);
				if (i < size - 1)
					strcpy(str + strlen(str), ", ");
				curr_p = strlen(str);
			}
			str[curr_p] = ']';
			str[curr_p + 1] = '\0';
			break;
		}
		case OBJECT:
			if (obj.type->operator_to_string != -1)
			{
				stack[(*sp)++] = obj;
				CallFunc(obj.type->operator_to_string);
				strcpy(str, stack[(*sp)-1].p->str);
				*sp -= 2;
				break;
			}
			sprintf(str, "<%s at 0x%x>", obj.type->name, obj.p->v); 
			break;
	}
}

void String_Add(Object *stack, int *sp)
{
	// Fetch operation data
    Object left = stack[(*sp)-2];
    Object right = stack[(*sp)-1];
    char right_str[80];
    AsString(right, right_str, stack, sp);

	// Create a new combined string
    int left_len = strlen(left.p->str);
    int right_len = strlen(right_str);
    char *str = (char*)malloc(left_len + right_len + 1);
    strcpy(str, left.p->str);
	strcpy(str + left_len, right.p->str);
	
	// Return the new string
	Object result;
	result.type = PrimType(STRING);
	result.p = AllocPointer(str);
	stack[(*sp)++] = result;
}

void String_Multiply(Object *stack, int *sp)
{
	// Fetch operation data
	Object left = stack[(*sp)-2];
    Object right = stack[(*sp)-1];
	if (right.type != PrimType(INT))
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
	result.type = PrimType(STRING);
	result.p = AllocPointer(str);
	stack[(*sp)++] = result;
}

void StringError(Object *stack, int *sp)
{
	printf("Error: invalid string operation\n");
}

void String(Object *stack, int *sp)
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

void String_Length(Object *stack, int *sp)
{
	Object str = stack[(*sp)-1];
	int length = strlen(str.p->str);

	Object size_obj;
	size_obj.type = PrimType(INT);
	size_obj.i = length;
	stack[(*sp)++] = size_obj;
}

void String_At(Object *stack, int *sp)
{
	char *str = stack[(*sp)-2].p->str;
	int index = stack[(*sp)-1].i;

	Object c_obj;
	c_obj.type = PrimType(CHAR);
	c_obj.c = str[index];
	stack[(*sp)++] = c_obj;
}

void String_Append(Object *stack, int *sp)
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

void RegisterString()
{
	RegisterFunc((char*)"AsString", String);
	RegisterFunc((char*)"String:Length", String_Length);
	RegisterFunc((char*)"String:At", String_At);
	RegisterFunc((char*)"String:Append", String_Append);
	RegisterFunc((char*)"String:operator_add", String_Add);
	RegisterFunc((char*)"String:operator_multiply", String_Multiply);
}
