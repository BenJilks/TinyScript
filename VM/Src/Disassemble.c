#include <stdio.h>
#include <string.h>

#define SINGLE (int)((char)fgetc(file))
//#define INT (SINGLE | (SINGLE << 8) | (SINGLE << 16) | (SINGLE << 24))
#define INT (SINGLE | (SINGLE >> 8) | (SINGLE >> 16) | (SINGLE >> 24))
#define FLOAT INT
#define STRING(str) \
{ \
    int size = SINGLE; \
    fread(str, 1, size, file); \
    str[size] = '\0'; \
}

static void DisFunc(FILE *file, int len)
{
    char c;
    int counter = 0;
    while ((c = SINGLE) != EOF)
    {
        counter++;
        char str[80];
        printf("    ");
        switch(c)
        {
            case 0: printf("ALLOC %i\n", SINGLE); counter += 1; break;
            case 1: printf("PUSH_ARG %i\n", SINGLE); counter += 1; break;
            case 2: printf("PUSH_LOC %i\n", SINGLE); counter += 1; break;
            case 3: printf("PUSH_INT %i\n", INT); counter += 4; break;
            case 4: printf("PUSH_FLOAT %i\n", FLOAT); counter += 4; break;
            case 5: printf("PUSH_CHAR '%c'\n", SINGLE); counter += 1; break;
            case 6: printf("PUSH_BOOL %s\n", SINGLE ? "true" : "false"); counter += 1; break;
            case 7: STRING(str); printf("PUSH_STRING \"%s\"\n", str); counter += strlen(str) + 1; break;
            case 8: printf("POP %i\n", SINGLE); counter += 1; break;
            case 9: printf("ASSIGN %i\n", SINGLE); counter += 1; break;
            case 10: printf("ADD\n"); break;
            case 11: printf("SUB\n"); break;
            case 12: printf("MUL\n"); break;
            case 13: printf("DIV\n"); break;
            case 14: printf("EQUALS\n"); break;
            case 15: printf("GREATERTHAN\n"); break;
            case 16: printf("GREATERTHANEQUAL"); break;
            case 17: printf("LESSTHAN\n"); break;
            case 18: printf("LESSTHANEQUAL"); break;
            case 19: printf("AND"); break;
            case 20: printf("OR"); break;
            case 21: printf("CALL %i with %i args\n", INT, SINGLE); counter += 5; break;
            case 22: printf("CALL_SYS %i with %i args\n", INT, SINGLE); counter += 5; break;
            case 23: printf("CALL_METHOD %i with %i args\n", INT, SINGLE); counter += 5; break;
            case 24: printf("RETURN\n"); break;
            case 25: printf("BRANCH_IF_NOT %i\n", INT); counter += 4; break;
            case 26: printf("BRANCH_IF_IT %i\n", INT); counter += 4; break;
            case 27: printf("BRANCH %i\n", INT); counter += 4; break;
            case 28: printf("INC_LOC %i %i\n", SINGLE, SINGLE); counter += 2; break;
            case 29: printf("POP_ARGS %i\n", SINGLE); counter += 1; break;
            case 30: printf("MALLOC %i\n", SINGLE); counter += 1; break;
            case 31: printf("PUSH_ATTR %i\n", SINGLE); counter += 1; break;
            case 32: printf("PUSH_INDEX\n"); break;
            case 33: printf("ASSIGN_ATTR %i\n", SINGLE); counter += 1; break;
            case 34: printf("ASSIGN_INDEX\n"); break;
            case 35: printf("MAKE_ARRAY %i\n", SINGLE); counter += 1; break;
            case 36: printf("MAKE_IT\n"); break;
            case 37: printf("IT_NEXT %i\n", SINGLE); counter += 1; break;
        }

        if (counter >= len)
            break;
    }
}

void Disassemble(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");

    // Skip over external info
    int i, j;
    int extern_len = SINGLE;
    for (i = 0; i < extern_len; i++)
    {
        fseek(file, SINGLE, SEEK_CUR);

        int call_len = SINGLE;
        for (j = 0; j < call_len; j++)
            fseek(file, SINGLE, SEEK_CUR);
    }

    // Skip over type info (string, 1 * char, 9 * int)
    int type_length = SINGLE;
    for (i = 0; i < type_length; i++)
    {
        int length = SINGLE;
        fseek(file, length + 1 + (4 * 9), SEEK_CUR);
    }

    int func_len = INT;
    for (i = 0; i < func_len; i++)
    {
        char name[80];
        STRING(name);
        int size = INT;
        int len = INT;

        printf("func %s [size: %i, length: %i]\n", name, size, len);
        DisFunc(file, len);
        printf("\n");
    }
}
