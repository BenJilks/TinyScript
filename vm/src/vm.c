#include "vm.h"
#include "bytecode.h"
#include "flags.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if DEBUG_VM
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) ;
#endif

typedef struct VMExternal
{
    char name[80];
    VMFunc func;
} VMExternal;

typedef struct VMLink
{
    int id;
    VMFunc func;
} VMLink;

static VMExternal exernals[80];
static int external_size = 0;

void vm_init()
{
    external_size = 0;
}

void register_external(const char *name, VMFunc func)
{
    strcpy(exernals[external_size].name, name);
    exernals[external_size].func = func;
    external_size += 1;
}

#define BYTE code[s.pc]
#define NBYTE code[s.pc++]
#define INT *(int*)(code + s.pc)

#define MAX(a, b) (a) > (b) ? (a) : (b)

#define OPERATION(ltype, op, rtype, restype) \
    { \
        LOG("%s %s %s = %s\n", #ltype, #op, #rtype, #restype); \
        ltype left = *(ltype*)(s.stack + s.sp - sizeof(ltype) - sizeof(rtype)); \
        rtype right = *(rtype*)(s.stack + s.sp - sizeof(rtype)); \
        restype res = left op right; \
        memcpy(s.stack + s.sp - sizeof(ltype) - sizeof(rtype), &res, sizeof(restype)); \
        s.sp += sizeof(restype) - sizeof(ltype) - sizeof(rtype); \
        break; \
    }

#define LOGIC_SET(types, left, right) \
    case BC_MORE_THAN_##types: OPERATION(left, >, right, char); \
    case BC_LESS_THAN_##types: OPERATION(left, <, right, char); \
    case BC_MORE_THAN_EQUALS_##types: OPERATION(left, >=, right, char); \
    case BC_LESS_THAN_EQUALS_##types: OPERATION(left, <=, right, char); \
    case BC_EQUALS_##types: OPERATION(left, ==, right, char);

#define OPERATION_SET(types, left, right, res) \
    case BC_ADD_##types: OPERATION(left, +, right, res); \
    case BC_SUB_##types: OPERATION(left, -, right, res); \
    case BC_MUL_##types: OPERATION(left, *, right, res); \
    case BC_DIV_##types: OPERATION(left, /, right, res); \
    LOGIC_SET(types, left, right)

#define CAST(name, from, to) \
    case BC_CAST_##name: \
        LOG("Cast from %s to %s\n", #from, #to); \
        *(to*)(s.stack + s.sp - sizeof(from)) = (to)*(from*)(s.stack + s.sp - sizeof(from)); \
        s.sp -= MAX(sizeof(from) - sizeof(to), 0); \
        break; \

#define CAST_SET(name, to) \
    CAST(INT_##name, int, to) \
    CAST(FLOAT_##name, float, to) \
    CAST(CHAR_##name, char, to) \
    CAST(BOOL_##name, char, to) \

int decode_header(char *data, VMLink **links, int *link_size)
{
    int pc = 0, i, j;
    int external_count = data[pc++];
    char name[80];

    *link_size = external_count;
    *links = malloc(external_count * sizeof(VMLink));

    for (i = 0; i < external_count; i++)
    {
        VMLink link;

        link.id = *(int*)(data + pc); pc += 4;
        int name_len = data[pc++];
        for (j = 0; j < name_len; j++)
            name[j] = data[pc++];
        name[name_len] = '\0';
        LOG("Searching for external '%s' with id %i", name, link.id);

        int found = 0;
        for (j = 0; j < external_size; j++)
        {
            if (!strcmp(exernals[j].name, name))
            {
                link.func = exernals[j].func;
                found = 1;
                break;
            }
        }

        (*links)[i] = link;
        if (!found)
        {
            LOG("\nError: Could not find external '%s'\n", name);
        }
        else
        {
            LOG(" [Found]\n");
        }
    }

    return pc;
}

int vm_run(char *data, int start, char *return_value)
{

    VMState s;
    s.pc = start;
    s.sp = 0;
    s.bp = 0;
    s.depth = 0;
    s.stack = (char*)malloc(STACK_MEMORY);

    VMLink *links;
    int link_size = 0;
    int code_start = decode_header(data, &links, &link_size);
    char *code = data + code_start;

    int running = 1;
    while (running)
    {
        LOG("%i: ", s.pc);

        switch(code[s.pc++])
        {
            case BC_PUSH_1: 
                LOG("push 1b %i\n", BYTE); 
                s.stack[s.sp++] = code[s.pc++]; 
                break;
            
            case BC_PUSH_4: 
                LOG("push 4b %i\n", INT); 
                memcpy(s.stack + s.sp, code + s.pc, 4);
                s.sp += 4; s.pc += 4; 
                break;
            
            case BC_PUSH_X:
                LOG("push %ib\n", BYTE);
                memcpy(s.stack + s.sp, code + s.pc + 1, BYTE);
                s.sp += BYTE; s.pc += NBYTE;
                break;
            
            case BC_POP:
                LOG("pop %ib\n", BYTE);
                s.sp -= code[s.pc++]; 
                break;
            
            case BC_ALLOC:
                LOG("alloc %ib\n", BYTE);
                s.sp += code[s.pc++]; 
                break;
            
            case BC_STORE_LOCAL_4:
                LOG("store 4b at %i\n", BYTE);
                memcpy(s.stack + s.bp + NBYTE, s.stack + s.sp - 4, 4); s.sp -= 4;
                break;

            case BC_STORE_LOCAL_X:
            {
                LOG("store %ib at %i\n", INT, code[s.pc + 4]);
                int size = INT; s.pc += 4;
                memcpy(s.stack + s.bp + NBYTE, s.stack + s.sp - size, size); s.sp -= size;
                break;
            }

            case BC_LOAD_LOCAL_4:
                LOG("load 4b at %i\n", BYTE);
                memcpy(s.stack + s.sp, s.stack + s.bp + NBYTE, 4); s.sp += 4;
                break;

            case BC_LOAD_LOCAL_X:
            {
                LOG("load %ib at %i\n", INT, code[s.pc + 4]);
                int size = INT; s.pc += 4;
                memcpy(s.stack + s.sp, s.stack + s.bp + NBYTE, size); s.sp += size;
                break;
            }

            case BC_LOCAL_REF:
            {
                LOG("return ref of local at %ib\n", BYTE);
                int loc = s.bp + NBYTE;
                memcpy(s.stack + s.sp, &loc, 4); s.sp += 4;
                break;
            }

            case BC_COPY:
            {
                LOG("copy %ib\n", BYTE);
                int loc = *(int*)(s.stack + s.sp - 4); s.sp -= 4;
                memcpy(s.stack + s.sp, s.stack + loc, BYTE);
                s.sp += NBYTE;
                break;
            }
            
            case BC_GET_ARRAY_INDEX:
            {
                LOG("Get array (size %ib, element size %ib) element at index\n", INT, *(int*)(code + s.pc + 4));
                int array_size = INT; s.pc += 4;
                int element_size = INT; s.pc += 4;
                int index = *(int*)(s.stack + s.sp - 4); s.sp -= 4;
                memcpy(s.stack + s.sp - array_size, 
                    s.stack + s.sp - array_size + element_size * index, 
                    element_size);
                s.sp -= array_size - element_size;
                break;
            }

            case BC_ASSIGN_REF_X:
            {
                LOG("Assign ref %ib\n", INT);
                int loc = *(int*)(s.stack + s.sp - 4); s.sp -= 4;
                memcpy(s.stack + loc, s.stack + s.sp - INT, INT); s.sp -= INT;
                s.pc += 4;
                break;
            }

            case BC_CREATE_FRAME:
                LOG("create stack frame of size %i\n", BYTE);
                memcpy(s.stack + s.sp, &s.bp, 4); s.sp += 4;
                s.bp = s.sp;
                s.sp += NBYTE;
                break;
            
            case BC_CALL:
                LOG("call function at %i\n", INT);
                memcpy(s.stack + s.sp, &s.pc, 4); s.sp += 4;
                s.pc = INT;
                s.depth++;
                break;
            
            case BC_CALL_EXTERNAL:
            {
                LOG("call external function %i\n", INT);
                int i;
                for (i = 0; i < link_size; i++)
                {
                    if (links[i].id == INT)
                    {
                        links[i].func(&s);
                        break;
                    }
                }
                s.pc += 4;
                break;
            }
            
            case BC_RETURN:
            {
                LOG("Return size %i with arg size %i\n", BYTE, code[s.pc+1]);
                int return_size = NBYTE;
                int arg_size = NBYTE;
                if (s.depth <= 0)
                {
                    memcpy(return_value, s.stack + s.sp - return_size, return_size);
                    running = 0;
                    break;
                }

                // Copy return value into correct slot
                memcpy(s.stack + s.bp - 8 - arg_size - return_size, 
                    s.stack + s.sp - return_size, return_size); 
                s.sp -= return_size;

                s.sp = s.bp;
                memcpy(&s.bp, s.stack + s.sp - 4, 4); s.sp -= 4;
                memcpy(&s.pc, s.stack + s.sp - 4, 4); s.sp -= 4;
                s.sp -= arg_size;
                s.pc += 4;
                s.depth--;
                break;
            }

            case BC_JUMP_IF_NOT:
                LOG("Jump if %i to %i\n", s.stack[s.sp-1], INT);
                if (!s.stack[--s.sp])
                    s.pc = INT;
                else
                    s.pc += 4;
                break;

            OPERATION_SET(INT_INT, int, int, int)
            OPERATION_SET(INT_FLOAT, int, float, float)
            OPERATION_SET(FLOAT_FLOAT, float, float, float)
            OPERATION_SET(FLOAT_INT, float, int, float)
            CAST_SET(INT, int)
            CAST_SET(FLOAT, float)
            CAST_SET(CHAR, char)
            CAST_SET(BOOL, char)

            default: 
                printf("Error: Unkown bytecode %i\n", code[s.pc-1]); 
                running = 0;
        }

#if DEBUG_STACK
        int i;
        printf("Stack: ");
        for (i = 0; i < s.sp; i++)
            printf("%i ", s.stack[i]);
        printf("\n");
#endif
    }

    free(s.stack);
    free(links);
    return 0;
}
