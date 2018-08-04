#include "TinyScript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Object Compair(Object left, Object right, VM vm)
{
	Object obj;
	switch(left.type->prim)
	{
		case INT:
			switch(right.type->prim)
			{
				case INT: obj.type = vm.PrimType(BOOL); obj.i = left.i == right.i; break;
				case FLOAT: obj.type = vm.PrimType(BOOL); obj.i = left.i == right.f; break;
				case CHAR: obj.type = vm.PrimType(BOOL); obj.i = left.i == right.c; break;
				default: break;
			}
			break;
		case FLOAT:
			switch(right.type->prim)
			{
				case INT: obj.type = vm.PrimType(BOOL); obj.i = left.f == right.i; break;
				case FLOAT: obj.type = vm.PrimType(BOOL); obj.i = left.f == right.f; break;
				case CHAR: obj.type = vm.PrimType(BOOL); obj.i = left.f == right.c; break;
				default: break;
			}
			break;
		case CHAR:
			switch(right.type->prim)
			{
				case INT: obj.type = vm.PrimType(BOOL); obj.i = left.c == right.i; break;
				case FLOAT: obj.type = vm.PrimType(BOOL); obj.i = left.c == right.f; break;
				case CHAR: obj.type = vm.PrimType(BOOL); obj.i = left.c == right.c; break;
				default: break;
			}
			break;
		case BOOL:
			switch(right.type->prim)
			{
				case BOOL: obj.type = vm.PrimType(BOOL); obj.i = left.c == right.c; break;
				default: break;
			}
			break;
		case STRING:
			switch(right.type->prim)
			{
				case STRING: obj.type = vm.PrimType(BOOL); obj.i = !strcmp(left.p->str, right.p->str); break;
				default: break;
			}
			break;
        case OBJECT:
            switch(right.type->prim)
			{
				case OBJECT: obj.type = vm.PrimType(BOOL); obj.i = left.p->v == right.p->v; break;
				default: break;
			}
            break;
	}
	return obj;
}

void Array_push(Object *stack, int *sp, VM vm)
{
    Object arr = stack[(*sp) - 2];
    Object obj = stack[(*sp) - 1];
    int size = arr.p->attrs[0].i;

    arr.p->v = realloc(arr.p->v, (size + 2) * sizeof(Object));
    arr.p->attrs[size + 1] = obj;
    arr.p->attrs[0].i++;
    stack[(*sp)++] = (Object){vm.PrimType(INT), 0};
}

void Array_size(Object *stack, int *sp, VM vm)
{
    Object arr = stack[(*sp) - 1];
    Object size_obj = arr.p->attrs[0];
    stack[(*sp)++] = size_obj;
}

void Array_operator_add(Object *stack, int *sp, VM vm)
{
    Object left = stack[(*sp) - 2];
    Object right = stack[(*sp) - 1];
    int left_size = left.p->attrs[0].i;
    int right_size = right.p->attrs[0].i;
    int new_size = left_size + right_size;

    Object *attrs = (Object*)malloc(new_size + 1);
    attrs[0] = (Object){vm.PrimType(INT), new_size};
    memcpy(attrs + 1, left.p->attrs + 1, sizeof(Object) * left_size);
    memcpy(attrs + left_size + 1, right.p->attrs + 1, sizeof(Object) * right_size);

    Object new_arr;
    new_arr.type = vm.PrimType(ARRAY);
    new_arr.p = vm.AllocPointer(attrs);
    stack[(*sp)++] = new_arr;
}

void Array_operator_multiply(Object *stack, int *sp, VM vm)
{
    Object left = stack[(*sp) - 2];
    Object right = stack[(*sp) - 1];
    if (right.type != vm.PrimType(INT))
	{
		printf("Error: cannot multiply an array by a '%s'\n", 
			right.type->name);
		return;
	}
    int size = left.p->attrs[0].i;
    int amount = right.i;
    int i;

    Object *attrs = (Object*)malloc((size * amount + 1) * sizeof(Object));
    attrs[0] = (Object){vm.PrimType(INT), size * amount};
    for (i = 0; i < amount; i++)
        memcpy(attrs + 1 + (i * size), left.p->attrs + 1, sizeof(Object) * size);
    
    Object new_arr;
    new_arr.type = vm.PrimType(ARRAY);
    new_arr.p = vm.AllocPointer(attrs);
    stack[(*sp)++] = new_arr;
}

void Array_operator_get_index(Object *stack, int *sp, VM vm)
{
    Object arr = stack[(*sp) - 2];
    Object index = stack[(*sp) - 1];
    Object obj = arr.p->attrs[index.i + 1];
    stack[(*sp)++] = obj;
}

void Array_operator_set_index(Object *stack, int *sp, VM vm)
{
    Object arr = stack[(*sp) - 3];
    Object obj = stack[(*sp) - 2];
    Object index = stack[(*sp) - 1];
    arr.p->attrs[index.i+1] = obj;

    stack[(*sp)++] = (Object){vm.PrimType(INT), 0};
}

int Find(Object arr, Object obj, VM vm)
{
    int i;
    int len = arr.p->attrs[0].i;
    for (i = 1; i < len + 1; i++)
    {
        Object item = arr.p->attrs[i];
        if (Compair(item, obj, vm).i)
            return i - 1;
    }

    return -1;
}

void Array_contains(Object *stack, int *sp, VM vm)
{
    Object arr = stack[(*sp) - 2];
    Object obj = stack[(*sp) - 1];

    int found = Find(arr, obj, vm) != -1;
    stack[(*sp)++] = (Object){vm.PrimType(BOOL), found};
}

void Array_index_of(Object *stack, int *sp, VM vm)
{
    Object arr = stack[(*sp) - 2];
    Object obj = stack[(*sp) - 1];

    int index = Find(arr, obj, vm);
    stack[(*sp)++] = (Object){vm.PrimType(INT), index};
}
