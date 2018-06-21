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

void StringAdd(Object *stack, int *sp)
{
    Object left = stack[(*sp)-2];
    Object right = stack[(*sp)-1];
    char right_str[80];
    AsString(right, right_str, stack, sp);

    int left_len = strlen(left.p->str);
    int right_len = strlen(right_str);
    left.p->v = realloc(left.p->v, left_len + right_len + 1);
    memcpy(left.p->str + left_len, right_str, right_len + 1);
}

void StringMultiply(Object *stack, int *sp)
{
	Object left = stack[(*sp)-2];
    Object right = stack[(*sp)-1];
	if (right.type != PrimType(INT))
	{
		printf("Error: cannot multiply a string by a '%s'\n", 
			right.type->name);
		return;
	}

	int left_len = strlen(left.p->str);
	left.p->v = realloc(left.p->v, left_len * right.i + 1);
	
	int i;
	for (i = 1; i < right.i; i++)
		memcpy(left.p->str + (i * left_len), left.p->v, left_len);
	left.p->str[left_len * right.i] = '\0';
}

void StringError(Object *stack, int *sp)
{
	printf("Error: invalid string operation\n");
}
