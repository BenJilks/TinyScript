#include <stdio.h>

#define SINGLE (int)fgetc(file)
#define INT (SINGLE | (SINGLE << 8) | (SINGLE << 16) | (SINGLE << 24))
#define FLOAT INT
#define STRING(str) \
{ \
    int size = SINGLE; \
    fread(str, 1, size, file); \
    str[size] = '\0'; \
}

void Disassemble(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    int start = INT;

    // Skip over syscall info
    int i;
    int syscall_length = SINGLE;
    for (i = 0; i < syscall_length; i++)
    {
        int length = SINGLE;
        fseek(file, length, SEEK_CUR);
    }

    // Skip over type info (string, 1 * char, 9 * int)
    int type_length = SINGLE;
    for (i = 0; i < type_length; i++)
    {
        int length = SINGLE;
        fseek(file, length + 1 + (4 * 9), SEEK_CUR);
    }

    char c;
    while ((c = fgetc(file)) != EOF)
    {
        char str[80];
        switch(c)
        {
            case 0: printf("ALLOC %i\n", SINGLE); break;
            case 1: printf("PUSH_ARG %i\n", SINGLE); break;
            case 2: printf("PUSH_LOC %i\n", SINGLE); break;
            case 3: printf("PUSH_INT %i\n", INT); break;
            case 4: printf("PUSH_FLOAT %i\n", FLOAT); break;
            case 5: printf("PUSH_CHAR '%c'\n", SINGLE); break;
            case 6: printf("PUSH_BOOL %s\n", SINGLE ? "true" : "false"); break;
            case 7: STRING(str); printf("PUSH_STRING \"%s\"\n", str); break;
            case 8: printf("POP %i\n", SINGLE); break;
            case 9: printf("ASSIGN %i\n", SINGLE); break;
            case 10: printf("ADD\n"); break;
            case 11: printf("SUB\n"); break;
            case 12: printf("MUL\n"); break;
            case 13: printf("DIV\n"); break;
            case 14: printf("EQUALS\n"); break;
            case 15: printf("GREATERTHAN\n"); break;
            case 16: printf("LESSTHAN\n"); break;
            case 17: printf("CALL %i\n", INT); break;
            case 18: printf("CALL_SYS %i\n", INT); break;
            case 19: printf("RETURN\n"); break;
            case 20: printf("BRANCH_IF_NOT %i\n", INT); break;
            case 21: printf("BRANCH_IF_IT %i\n", INT); break;
            case 22: printf("BRANCH %i\n", INT); break;
            case 23: printf("INC_LOC %i %i\n", SINGLE, SINGLE); break;
            case 24: printf("POP_ARGS %i\n", SINGLE); break;
            case 25: printf("MALLOC %i\n", SINGLE); break;
            case 26: printf("PUSH_ATTR %i\n", SINGLE); break;
            case 27: printf("PUSH_INDEX\n"); break;
            case 28: printf("ASSIGN_ATTR %i\n", SINGLE); break;
            case 29: printf("ASSIGN_INDEX\n"); break;
            case 30: printf("MAKE_ARRAY %i\n", SINGLE); break;
            case 31: printf("MAKE_IT\n"); break;
            case 32: printf("IT_NEXT %i\n", SINGLE); break;
        }
    }
}
