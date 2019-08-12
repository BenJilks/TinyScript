#include "bytecode.h"
#include <stdio.h>

static int decode_header(char *data)
{
    int pc = 0, i, j;
    int external_count = data[pc++];
    char name[80];

    for (i = 0; i < external_count; i++)
    {
        int id = *(int*)(data + pc); pc += 4;
        int name_len = data[pc++];
        for (j = 0; j < name_len; j++)
            name[j] = data[pc++];
        name[name_len] = '\0';
        printf("External '%s' with id %i\n", name, id);
    }

    return pc;
}

void disassemble(char *code, int size)
{
    int i = decode_header(code);
    int j = 0;
    int start = i;

    printf("%i / %i\n", start, size);
    while (i < size)
    {
        int bytecode = code[i++];
        int code_size = bytecode_size[bytecode];
        if (code_size == -1)
            code_size = code[i++];

        printf("%i  %s (", i - start - 1, bytecode_names[bytecode]);
        for (j = 0; j < code_size; j++)
            printf("%i%s", code[i++], j == code_size-1 ? "" : ", ");
        printf(")\n");
    }
}
