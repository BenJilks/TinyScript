#include <stdlib.h>
#include <stdio.h>
#include "io.h"
#include "vm.h"

// Zero value object
static struct VMObject zero;

static char *ToStr(struct VMObject obj)
{
    char *str = malloc(80);
    switch(obj.type->prim_type)
    {
        case PRIM_INT: sprintf(str, "%i", obj.i); break;
        case PRIM_FLOAT: sprintf(str, "%.6g", obj.f); break;
        case PRIM_CHAR: sprintf(str, "%c", obj.c); break;
        case PRIM_BOOL: sprintf(str, "%s", obj.c ? "true" : "false"); break;
        case PRIM_STRING: sprintf(str, "%s", obj.p->str); break;
        case PRIM_OBJECT: sprintf(str, "<%s at 0x%x>", obj.type->name, obj.p->p); break;
    }
    return str;
}

struct VMObject Print(struct VMObject *args, int arg_size)
{
    int i;
    for (i = 0; i < arg_size; i++)
    {
        char *str = ToStr(args[i]);
        printf("%s", str);
        free(str);
    }

    return zero;
}

struct VMObject Log(struct VMObject *args, int arg_size)
{
    Print(args, arg_size);
    printf("\n");
    return zero;
}

void VM_Load_IO()
{
    zero.type = VM_PrimType(PRIM_INT);
    zero.i = 0;
    
    struct VMMod *io = VM_CreateSysMod("io");
    VM_LoadSysFunc(io, Print, "print");
    VM_LoadSysFunc(io, Log, "log");
}
