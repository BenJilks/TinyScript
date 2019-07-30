#include "std.h"
#include "vm.h"

static void log_int(VMState *state)
{
    int i = *(char*)(state->stack + state->sp - 4);
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
    printf("%s\n", state->stack + ref);
    state->sp -= 4;
}

void register_io()
{
    register_external("log(int)", log_int);
    register_external("log(float)", log_float);
    register_external("log(char)", log_char);
    register_external("log(bool)", log_bool);
    register_external("log(char ref)", log_raw_string);
}
