#ifndef VM_H
#define VM_H

enum PrimType
{
    PRIM_INT,
    PRIM_FLOAT,
    PRIM_CHAR,
    PRIM_BOOL,
    PRIM_OBJECT
};

struct VMType
{
    char name[80];
    int prim_type;
    int size;
};

struct VMObject
{
    struct VMType *type;
    union
    {
        int i;
        float f;
        char c, b;
    };
};

typedef struct VMObject (*SysFunc)(struct VMObject *args, int arg_size);
struct VMMod;
struct VMFunc
{
    char name[80];
    char *code;
    int size;
    struct VMMod *mod;

    char is_sys;
    SysFunc sys_func;
};

struct VMSubMod
{
    char name[80];
    char func_names[80][80];
    struct VMFunc *funcs[80];
    int func_size;
};

struct VMMod
{
    char name[80];
    struct VMFunc funcs[80];
    struct VMSubMod sub_mods[80];
    int func_size, sub_size;
    char *data;
};

struct VMMod *VM_LoadMod(char *header, char *data);
int VM_Link();

struct VMObject VM_CallFunc(struct VMFunc *func, 
    struct VMObject *args, int arg_size);
struct VMObject VM_CallFuncName(const char *func_name);

#endif // VM_H
