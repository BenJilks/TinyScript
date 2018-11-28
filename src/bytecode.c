#include "bytecode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INT(d, p) *(int*)(d + p)
#define STRING(d, p) { int l = d[p++]; memcpy(str, d+p, l); \
    str[l] = '\0'; p += l; }

void decode_header(char *header)
{
    char str[80];
    int i, pc = 0;
    int func_size = INT(header, pc); pc += 4;
    for (i = 0; i < func_size; i++)
    {
        STRING(header, pc);
        int start = INT(header, pc); pc += 4;
        int size = INT(header, pc); pc += 4;
        printf("Found function '%s' at %i of size %i\n", str, start, size);
    }
}

void disassemble(char *code, int len)
{
    char str[80];
    int pc = 0;

    while (pc < len)
    {
        char bc = code[pc++];
        printf("%i (%x): ", pc, bc);
        switch(bc)
        {
            case BC_PUSH_INT: printf("Push int %i\n", INT(code, pc)); pc += 4; break;
            case BC_PUSH_FLOAT: printf("Push float %d\n", INT(code, pc)); pc += 4; break;
            case BC_PUSH_CHAR: printf("Push char %i\n", code[pc++]); break;
            case BC_PUSH_BOOL: printf("Push bool %s\n", code[pc++] ? "true" : "false"); break;
            case BC_PUSH_STRING: STRING(code, pc); printf("Push string '%s'\n", str); break;
            case BC_PUSH_ARG: printf("Push arg at %i\n", INT(code, pc)); pc += 4; break;
            case BC_PUSH_LOC: printf("Push loc at %i\n", INT(code, pc)); pc += 4; break;
            case BC_ASSIGN_LOC: printf("Assign to loc %i\n", INT(code, pc)); pc += 4; break;

            case BC_CALL: printf("Call with %i args at %i\n", INT(code, pc), INT(code, pc+4)); pc += 8; break;
            case BC_CALL_MOD: printf("Call with %i args in mod %i at %i\n", INT(code, pc), INT(code, pc+4), INT(code, pc+8)); pc += 12; break;
            case BC_RETURN: printf("Return\n"); break;
            case BC_JUMP_IF_NOT: printf("Jump of not to %i\n", INT(code, pc)); pc += 4; break;
            case BC_JUMP: printf("Jump to %i\n", INT(code, pc)); pc += 4; break;
            case BC_POP: printf("Pop %i element(s) from stack\n", code[pc++]); break;

            case BC_ADD: printf("Add\n"); break;
            case BC_SUB: printf("Sub\n"); break;
            case BC_MUL: printf("Mul\n"); break;
            case BC_DIV: printf("Div\n"); break;
            case BC_MORE_THAN: printf("MoreThan\n"); break;
            case BC_LESS_THAN: printf("LessThan\n"); break;
            case BC_ASSIGN: printf("Assign\n"); break;
        }
    }
}
