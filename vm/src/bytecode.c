#include "bytecode.h"
#include <stdio.h>

void disassemble(char *code, int size)
{
    int i = 0;
    while (i < size)
    {
        int bytecode = code[i++];
        i += bytecode_size[bytecode];
        printf("%s\n", bytecode_names[bytecode]);
    }
}
