#ifndef COMPILER_H
#define COMPILER_H

#include "tokenizer.h"
#include "symbol.h"
#define BUFFER_SIZE 100
#define MAX_LABELS 100

struct Function
{
    char name[80];
    int start;
    int size;
};

struct SubType
{
    char name[80];
    char attr_names[80][80];
    int attr_size;
};

struct SubModule
{
    char name[80];
    char func_names[80][80];
    struct SubType types[80];
    int func_size, type_size;
};

struct Label
{
    int uses[80];
    int use_count;
    int addr;
};

struct Module
{
    // Module data
    char name[80];
    struct SymbolTable table;
    struct Tokenizer *tk;

    // Current function
    struct Label *labels;
    int label_counter;
    int loc, param, start_addr;

    // Final output
    char *code;
    char *header;
    int cp, cbuffer;
    
    // Module data
    struct Function *funcs;
    struct SubModule *mods;
    int func_size, mod_size;
};

struct Module create_module(const char *name, struct Tokenizer *tk);
struct SubType create_sub_type(const char *name);
int find_mod_func_location(struct SubModule *sub, const char *func_name);
void delete_module(struct Module mod);

void gen_char(struct Module *mod, char ins);
void gen_int(struct Module *mod, int i);
void gen_string(struct Module *mod, const char *str);
void parse_function(struct Module *mod, struct Tokenizer *tk);
void compile_module(struct Module *mod);

#endif // COMPILER_H
