#ifndef COMPILER_H
#define COMPILER_H

#include "tokenizer.h"
#include "symbol.h"
#define BUFFER_SIZE 100

struct Function
{
    char name[80];
    int start;
    int size;
};

struct SubModule
{
    char name[80];
    char func_names[80][80];
    int func_size;
};

struct Module
{
    char name[80];
    int loc, param;
    struct SymbolTable table;
    struct Tokenizer *tk;

    struct Function funcs[80];
    struct SubModule mods[80];
    char *code;
    char *header;
    int cp, cbuffer;
    int func_size, mod_size;
};

struct Module create_module(const char *name, struct Tokenizer *tk);
int find_mod_func_location(struct SubModule *sub, const char *func_name);
void delete_module(struct Module mod);

void gen_char(struct Module *mod, char ins);
void gen_int(struct Module *mod, int i);
void gen_string(struct Module *mod, const char *str);
void parse_function(struct Module *mod, struct Tokenizer *tk);
void compile_module(struct Module *mod);

#endif // COMPILER_H
