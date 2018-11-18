#include "vm.h"
#include "bytecode.h"
#include "flags.h"
#include <string.h>
#include <stdio.h>

// Define prim types
static struct VMType dt_int = {"int", PRIM_INT, -1};
static struct VMType dt_float = {"float", PRIM_FLOAT, -1};
static struct VMType dt_char = {"char", PRIM_CHAR, -1};
static struct VMType dt_bool = {"bool", PRIM_BOOL, -1};
static struct VMType dt_string = {"String", PRIM_OBJECT, 1};

#define INT(d, p) *(int*)(d + p)
#define FLOAT(d, p) *(float*)(d + p)
#define NEXT_INT(p) p += sizeof(int)
#define NEXT_FLOAT(p) p += sizeof(float)
#define STRING(str, d, p) { int l = d[p++]; memcpy(str, d + p, l); \
    str[l] = '\0'; p += l; }

// Currently loaded mods
static struct VMMod mods[80];
static int mod_size = 0;

static struct VMMod *FindMod(const char *name)
{
    int i;
    for (i = 0; i < mod_size; i++)
    {
        struct VMMod *mod = &mods[i];
        if (!strcmp(mod->name, name))
            return mod;
    }
    return NULL;
}

// Load a module from header and data
struct VMMod *VM_LoadMod(char *header, char *data)
{
    struct VMMod *mod = &mods[mod_size];
    int pc = 0, i, j;
    mod->data = data;

    // Load mods name
    STRING(mod->name, header, pc);
    if (FindMod(mod->name) != NULL)
    {
        // If a mod with this name has already been loaded
        printf("Error: A module named '%s' has already been loaded", 
            mod->name);
        return NULL;
    }
    LOG("Loading mod '%s'\n", mod->name);

    // Load mod functions
    mod->func_size = INT(header, pc); NEXT_INT(pc);
    LOG("Loading %i functions\n", mod->func_size);
    for (i = 0; i < mod->func_size; i++)
    {
        struct VMFunc func;
        STRING(func.name, header, pc);
        func.code = data + INT(header, pc); NEXT_INT(pc);
        func.size = INT(header, pc); NEXT_INT(pc);
        func.mod = mod;
        func.is_sys = 0;
        mod->funcs[i] = func;
        LOG("   - Loaded function '%s' at %i of size %i\n", 
            func.name, func.code - data, func.size);
    }

    // Load the other mods this mod uses
    mod->sub_size = INT(header, pc); NEXT_INT(pc);
    LOG("Loading %i sub mods\n", mod->sub_size);
    for (i = 0; i < mod->sub_size; i++)
    {
        struct VMSubMod sub;
        STRING(sub.name, header, pc);
        LOG("    - Loading sub mod '%s'\n", sub.name);

        // Load functions in sub mod
        sub.func_size = INT(header, pc); NEXT_INT(pc);
        for (j = 0; j < sub.func_size; j++)
        {
            STRING(sub.func_names[j], header, pc);
            LOG("       - Loaded sub func '%s'\n", sub.func_names[j]);
        }
        mod->sub_mods[i] = sub;
    }
    LOG("Finished loading mod\n\n");

    mod_size++;
    return mod;
}

struct VMMod *VM_CreateSysMod(const char *name)
{
    struct VMMod *mod = &mods[mod_size++];
    strcpy(mod->name, name);
    mod->data = NULL;
    mod->func_size = 0;
    mod->sub_size = 0;
    return mod;
}

void VM_LoadSysFunc(struct VMMod *mod, SysFunc func, const char *name)
{
    struct VMFunc sfunc;
    strcpy(sfunc.name, name);
    sfunc.sys_func = func;
    sfunc.is_sys = 1;
    mod->funcs[mod->func_size++] = sfunc;
}

struct VMType *VM_PrimType(int prim)
{
    switch(prim)
    {
        case PRIM_INT: return &dt_int;
        case PRIM_FLOAT: return &dt_float;
        case PRIM_CHAR: return &dt_char;
        case PRIM_BOOL: return &dt_bool;
    }
    return NULL;
}

// Find a function of a name within a module
static struct VMFunc *FindFuncInMod(struct VMMod *mod, const char *name)
{
    int i;
    for (i = 0; i < mod->func_size; i++)
        if (!strcmp(mod->funcs[i].name, name))
            return &mod->funcs[i];
    return NULL;
}

// Find all the function pointer within a sub mod from the real mod
static int LinkSubMod(struct VMSubMod *sub, struct VMMod *other)
{
    int has_error = 0;
    int i;
    for (i = 0; i < sub->func_size; i++)
    {
        struct VMFunc *func = FindFuncInMod(other, sub->func_names[i]);
        if (func == NULL)
        {
            printf("Error: no function called '%s' in module '%s'\n", 
                sub->func_names[i], sub->name);
            has_error = 1;
            continue;
        }
        sub->funcs[i] = func;
    }

    return has_error;
}

// Link all loaded mods together
int VM_Link()
{
    LOG("Linking...\n");
    int has_error = 0;
    int i, j;
    for (i = 0; i < mod_size; i++)
    {
        // For each mod, find the pointers to the needed mods
        struct VMMod *mod = &mods[i];
        for (j = 0; j < mod->sub_size; j++)
        {
            // Find loaded mods
            struct VMSubMod *sub = &mod->sub_mods[j];
            struct VMMod *other = FindMod(sub->name);
            if (other == NULL)
            {
                printf("Error: no module named '%s' found\n", 
                    sub->name);
                has_error = 1;
                continue;
            }

            // Link functions in mod
            if (LinkSubMod(sub, other))
                has_error = 1;
        }
    }
    LOG("Finished linking\n\n");

    return has_error;
}

#define CREATE_OBJECT(dtype, data, value) \
{ \
    struct VMObject obj; \
    obj.type = dtype; \
    obj.data = value; \
    stack[sp++] = obj; \
}

#define INVALID_OP(left, op, right) \
    printf("Error: Invalid operation %s %s %s\n", \
        left.type->name, op, right.type->name)

#define OPERATION_FUNC(name, op, op_name) \
    static struct VMObject name(struct VMObject left, struct VMObject right) \
    { \
        struct VMObject o; \
        switch(left.type->prim_type) \
        { \
            case PRIM_INT: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.type = &dt_int; o.i = left.i op right.i; break; \
                    case PRIM_FLOAT: o.type = &dt_float; o.f = left.i op right.f; break; \
                    case PRIM_CHAR: o.type = &dt_int; o.i = left.i op right.c; break; \
                    case PRIM_BOOL: \
                    default: INVALID_OP(left, op_name, right); break; \
                } \
                break; \
             \
            case PRIM_FLOAT: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.type = &dt_float; o.f = left.f op right.i; break; \
                    case PRIM_FLOAT: o.type = &dt_float; o.f = left.f op right.f; break; \
                    case PRIM_CHAR: o.type = &dt_float; o.f = left.f op right.c; break; \
                    case PRIM_BOOL:  \
                    default: INVALID_OP(left, op_name, right); break; \
                } \
                break; \
             \
            case PRIM_CHAR: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.type = &dt_int; o.i = left.c op right.i; break; \
                    case PRIM_FLOAT: o.type = &dt_float; o.f = left.c op right.f; break; \
                    case PRIM_CHAR: o.type = &dt_char; o.c = left.c op right.c; break; \
                    case PRIM_BOOL:  \
                    default: INVALID_OP(left, op_name, right); break; \
                } \
                break; \
             \
            case PRIM_BOOL: \
                INVALID_OP(left, op_name, right); \
                break; \
        } \
         \
        return o; \
    }

#define COMP_FUNC(name, op) \
    static struct VMObject name(struct VMObject left, struct VMObject right) \
    { \
        struct VMObject o; \
        o.type = &dt_bool; \
         \
        switch(left.type->prim_type) \
        { \
            case PRIM_INT: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.b = left.i op right.i; break; \
                    case PRIM_FLOAT: o.b = left.i op right.f; break; \
                    case PRIM_CHAR: o.b = left.i op right.c; break; \
                    case PRIM_BOOL: o.b = left.i op right.b; break; \
                } \
                break; \
             \
            case PRIM_FLOAT: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.b = left.f op right.i; break; \
                    case PRIM_FLOAT: o.b = left.f op right.f; break; \
                    case PRIM_CHAR: o.b = left.f op right.c; break; \
                    case PRIM_BOOL: o.b = left.f op right.b; break; \
                } \
                break; \
             \
            case PRIM_CHAR: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.b = left.c op right.i; break; \
                    case PRIM_FLOAT: o.b = left.c op right.f; break; \
                    case PRIM_CHAR: o.b = left.c op right.c; break; \
                    case PRIM_BOOL: o.b = left.c op right.b; break; \
                } \
                break; \
             \
            case PRIM_BOOL: \
                switch(right.type->prim_type) \
                { \
                    case PRIM_INT: o.b = left.b op right.i; break; \
                    case PRIM_FLOAT: o.b = left.b op right.f; break; \
                    case PRIM_CHAR: o.b = left.b op right.c; break; \
                    case PRIM_BOOL: o.b = left.b op right.b; break; \
                } \
                break; \
        } \
        return o; \
    }

#define OPERATION(bc, op, op_name) \
    case bc: \
        stack[sp-2] = op(stack[sp-2], stack[sp-1]); sp--; \
        LOG("%s\n", op_name); \
        break

OPERATION_FUNC(Add, +, "+");
OPERATION_FUNC(Sub, -, "-");
OPERATION_FUNC(Mul, *, "*");
OPERATION_FUNC(Div, /, "/");
COMP_FUNC(MoreThan, >);
COMP_FUNC(LessThan, <);
COMP_FUNC(EqualTo, ==);

// Call a function in a module
struct VMObject VM_CallFunc(struct VMFunc *func,
    struct VMObject *args, int arg_size)
{
    if (func->is_sys)
        return func->sys_func(args, arg_size);
    LOG("Calling function %s\n", func->name);

    struct VMObject stack[80];
    register char *code = func->code;
    register int sp = func->size;
    register int pc = 0;
    for (;;)
    {
        LOG("%i (%x): ", pc, code[pc]);
        switch (code[pc++])
        {
            // PUSH_INT <int>
            case BC_PUSH_INT: 
                CREATE_OBJECT(&dt_int, i, INT(code, pc)); 
                LOG("Push int %i\n", INT(code, pc)); 
                NEXT_INT(pc); 
                break;
            
            // PUSH_FLOAT <float>
            case BC_PUSH_FLOAT: 
                CREATE_OBJECT(&dt_float, f, FLOAT(code, pc)); 
                LOG("Push float %d\n", FLOAT(code, pc)); 
                NEXT_FLOAT(pc); 
                break;
            
            // PUSH_CHAR <char>
            case BC_PUSH_CHAR: 
                CREATE_OBJECT(&dt_char, c, code[pc++]); 
                LOG("Push char %i\n", code[pc - 1]); 
                break;
            
            // PUSH_BOOL <bool>
            case BC_PUSH_BOOL: 
                CREATE_OBJECT(&dt_bool, b, code[pc++]); 
                LOG("Push bool %s\n", code[pc - 1] ? "true" : "false");
                break;
            
            // PUSH_ARG <arg>
            case BC_PUSH_ARG:
                stack[sp++] = args[INT(code, pc)];
                LOG("Push argument at %i\n", INT(code, pc));
                pc += 4;
                break;
            
            // PUSH_LOC <loc>
            case BC_PUSH_LOC:
                stack[sp++] = stack[INT(code, pc)];
                LOG("Push argument at %i\n", INT(code, pc));
                pc += 4;
                break;
            
            // ASSIGN_LOC <loc>
            case BC_ASSIGN_LOC:
                stack[INT(code, pc)] = stack[--sp];
                LOG("Assign to loc %i\n", INT(code, pc));
                pc += 4;
                break;

            // CALL <arg_size> <func_id>
            case BC_CALL:
            {
                int arg_size = INT(code, pc);
                LOG("Call function %i\n", INT(code, pc+4));
                stack[sp-arg_size] = VM_CallFunc(&func->mod->funcs[INT(code, pc+4)], 
                    &stack[sp - arg_size], arg_size);
                sp -= arg_size - 1;
                pc += 8;
                break;
            }

            // CALL_MOD <arg_size> <mod_id> <func_id>
            case BC_CALL_MOD:
            {
                int arg_size = INT(code, pc);
                struct VMSubMod sub = func->mod->sub_mods[INT(code, pc+4)];
                LOG("Call function %i in mod %s\n", INT(code, pc+8), sub.name);
                stack[sp-arg_size] = VM_CallFunc(sub.funcs[INT(code, pc+8)], 
                    &stack[sp - arg_size], arg_size);
                sp -= arg_size - 1;
                pc += 12;
                break;
            }

            // JUMP_IF_NOT <to>
            case BC_JUMP_IF_NOT:
                LOG("Jump to %i if not\n", INT(code, pc));
                if (!stack[--sp].b) pc = INT(code, pc);
                else pc += 4;
                break;

            // POP <amount>
            case BC_POP:
                sp -= code[pc++];
                LOG("Pop %i\n", code[pc-1]);
                break;
            
            // Operations
            OPERATION(BC_ADD, Add, "Add");
            OPERATION(BC_SUB, Sub, "Sub");
            OPERATION(BC_MUL, Mul, "Mul");
            OPERATION(BC_DIV, Div, "Div");
            OPERATION(BC_MORE_THAN, MoreThan, "MoreThan");
            OPERATION(BC_LESS_THAN, LessThan, "LessThan");
            OPERATION(BC_EQUAL_TO, EqualTo, "EqualTo");
            
            // ASSIGN
            case BC_ASSIGN:
                stack[stack[sp-2].i] = stack[sp-1];
                sp -= 2;
                LOG("Assign\n");
                break;
            
            // RETURN
            case BC_RETURN:
                LOG("Return\n");
                return stack[sp-1];

            default: 
                printf("Error unkown bytecode %x (%i)\n", code[pc-1], code[pc-1]);
                break;
        }
    }
}

struct VMObject VM_CallFuncName(const char *func_name)
{
    struct VMFunc *func = NULL;

    // Find function with that name in all loaded modules
    int i, j;
    for (i = 0; i < mod_size; i++)
    {
        struct VMMod *mod = &mods[i];
        for (j = 0; j < mod->func_size; j++)
        {
            if (!strcmp(mod->funcs[j].name, func_name))
            {
                func = &mod->funcs[j];
                break;
            }
        }

        if (func != NULL)
            break;
    }

    // If the function was not found, throw an error
    if (func == NULL)
    {
        printf("Error: No function called '%s' found\n",
            func_name);
        
        struct VMObject zero = {&dt_int, 0};
        return zero;
    }
    return VM_CallFunc(func, NULL, 0);
}
