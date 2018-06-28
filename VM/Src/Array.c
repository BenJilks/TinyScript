#include "Array.h"
#include "VM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Array_Push(Object *stack, int *sp)
{
    Object arr = stack[(*sp) - 2];
    Object obj = stack[(*sp) - 1];
    int size = arr.p->attrs[0].i;

    arr.p->v = realloc(arr.p->v, size * sizeof(Object) + 1);
    arr.p->attrs[size + 1] = obj;
    arr.p->attrs[0].i++;
    stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void Array_Length(Object *stack, int *sp)
{
    Object arr = stack[(*sp) - 1];
    Object size_obj = arr.p->attrs[0];
    stack[(*sp)++] = size_obj;
}

void Array_Add(Object *stack, int *sp)
{
    Object left = stack[(*sp) - 2];
    Object right = stack[(*sp) - 1];
    int left_size = left.p->attrs[0].i;
    int right_size = right.p->attrs[0].i;
    int new_size = left_size + right_size;

    Object *attrs = (Object*)malloc(new_size + 1);
    attrs[0] = (Object){PrimType(INT), new_size};
    memcpy(attrs + 1, left.p->attrs + 1, sizeof(Object) * left_size);
    memcpy(attrs + left_size + 1, right.p->attrs + 1, sizeof(Object) * right_size);

    Object new_arr;
    new_arr.type = PrimType(ARRAY);
    new_arr.p = AllocPointer(attrs);
    stack[(*sp)++] = new_arr;
}

void Array_Multiply(Object *stack, int *sp)
{
    Object left = stack[(*sp) - 2];
    Object right = stack[(*sp) - 1];
    if (right.type != PrimType(INT))
	{
		printf("Error: cannot multiply an array by a '%s'\n", 
			right.type->name);
		return;
	}
    int size = left.p->attrs[0].i;
    int amount = right.i;
    int i;

    Object *attrs = (Object*)malloc((size * amount + 1) * sizeof(Object));
    attrs[0] = (Object){PrimType(INT), size * amount};
    for (i = 0; i < amount; i++)
        memcpy(attrs + 1 + (i * size), left.p->attrs + 1, sizeof(Object) * size);
    
    Object new_arr;
    new_arr.type = PrimType(ARRAY);
    new_arr.p = AllocPointer(attrs);
    stack[(*sp)++] = new_arr;
}

void Array_Get_Index(Object *stack, int *sp)
{
    Object arr = stack[(*sp) - 2];
    Object index = stack[(*sp) - 1];
    Object obj = arr.p->attrs[index.i + 1];
    stack[(*sp)++] = obj;
}

void Array_Set_Index(Object *stack, int *sp)
{
    Object obj = stack[(*sp) - 3];
    Object arr = stack[(*sp) - 2];
    Object index = stack[(*sp) - 1];
    arr.p->attrs[index.i+1] = obj;

    stack[(*sp)++] = (Object){PrimType(INT), 0};
}

void RegisterArray()
{
    RegisterFunc((char*)"List:push", Array_Push);
    RegisterFunc((char*)"List:size", Array_Length);
    RegisterFunc((char*)"List:operator_add", Array_Add);
    RegisterFunc((char*)"List:operator_multiply", Array_Multiply);
    RegisterFunc((char*)"List:operator_get_index", Array_Get_Index);
    RegisterFunc((char*)"List:operator_set_index", Array_Set_Index);
}
