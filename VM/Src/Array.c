#include "Array.h"
#include "VM.h"

void Array_Push(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
    Object arr = stack[(*sp) - 2];
    Object obj = stack[(*sp) - 1];

    
}

void Array_Length(Object *stack, int *sp, Pointer *pointers, int *pointer_count)
{
    
}

void RegisterArray()
{
    RegisterFunc((char*)"Array:Push", Array_Push);
    RegisterFunc((char*)"Array:Length", Array_Length);
}
