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

void Array_Get_Index(Object *stack, int *sp)
{
    Object arr = stack[(*sp) - 2];
    Object index = stack[(*sp) - 1];
    Object obj = arr.p->attrs[index.i + 1];
    stack[(*sp)++] = obj;
}

void RegisterArray()
{
    RegisterFunc((char*)"Array:Push", Array_Push);
    RegisterFunc((char*)"Array:Size", Array_Length);
    RegisterFunc((char*)"Array:operator_add", Array_Add);
    RegisterFunc((char*)"Array:operator_get_index", Array_Get_Index);
}
