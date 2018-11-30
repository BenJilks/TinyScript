#ifndef VM_H
#define VM_H

enum PrimType
{
    PRIM_INT,
    PRIM_FLOAT,
    PRIM_CHAR,
    PRIM_BOOL,
    PRIM_STRING,
    PRIM_OBJECT
};

struct VMType
{
    char name[80];
    int prim_type;
    int size;
};

struct VMObject;
struct VMPointer
{
    int counter;
    union
    {
        void *p;
        char *str;
        struct VMObject *arr;
    };
};

struct VMHeap
{
    struct VMPointer *mem;
    int mem_size;
};

struct VMObject
{
    struct VMType *type;
    union
    {
        int i;
        float f;
        char c, b;
        struct VMPointer *p;
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
    struct VMType *types[80];
    int func_size, type_size;
};

struct VMMod
{
    char name[80];
    struct VMFunc funcs[80];
    struct VMType types[80];
    struct VMSubMod sub_mods[80];
    int func_size, type_size, sub_size;
    char *data;
};

struct VMMod *VM_LoadMod(char *header, char *data);
struct VMMod *VM_CreateSysMod(const char *name);
void VM_LoadSysFunc(struct VMMod *mod, SysFunc func, const char *name);
struct VMType *VM_PrimType(int prim);
struct VMHeap VM_CreateHeap(int start_size);
int VM_Link();

struct VMObject VM_CallFunc(struct VMFunc *func, int arg_loc, 
    int arg_size, struct VMObject *stack, struct VMHeap *heap);
struct VMObject VM_CallFuncName(const char *func_name);

#endif // VM_H
