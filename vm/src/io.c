#include "std.h"
#include "vm.h"
#include <stdio.h>

static void log_int(VMState *state)
{
    int i = *(int*)(state->stack + state->sp - 4);
    printf("%i\n", i);
    state->sp -= 4;
}

static void log_float(VMState *state)
{
    float f = *(float*)(state->stack + state->sp - 4);
    printf("%.06g\n", f);
    state->sp -= 4;
}

static void log_char(VMState *state)
{
    printf("%c\n", state->stack[--state->sp]);
}

static void log_bool(VMState *state)
{
    printf("%s\n", state->stack[--state->sp] ? "true" : "false");
}

static void log_raw_string(VMState *state)
{
    int ref = *(char*)(state->stack + state->sp - 4);
    int len = *(char*)(state->stack + state->sp - 8);
    printf("%s\n", state->stack + ref);
    state->sp -= 8;
}

#define LOG_ARRAY_FUNC(name, type, printfunc) \
    static void name(VMState *state) \
    { \
        int ref = *(char*)(state->stack + state->sp - 4); \
        int len = *(char*)(state->stack + state->sp - 8); \
        printf("["); \
        for (int i = 0; i < len; i++) \
        { \
            printf(printfunc "%s", *(type*)(state->stack + ref + i * sizeof(type)), \
                i == len-1 ? "" : ", "); \
        } \
        printf("]\n"); \
        state->sp -= 8; \
    }

LOG_ARRAY_FUNC(log_int_array, int, "%i")
LOG_ARRAY_FUNC(log_float_array, float, "%.06g")
LOG_ARRAY_FUNC(log_bool_array, char, "%i")


void register_io()
{
    register_external("io.log(int)", log_int);
    register_external("io.log(float)", log_float);
    register_external("io.log(char)", log_char);
    register_external("io.log(bool)", log_bool);
    register_external("io.log(int ref, int)", log_int_array);
    register_external("io.log(float ref, int)", log_float_array);
    register_external("io.log(char ref, int)", log_raw_string);
    register_external("io.log(bool ref, int)", log_bool_array);
}
