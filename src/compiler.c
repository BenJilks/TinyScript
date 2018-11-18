#include "compiler.h"
#include "expression.h"
#include "bytecode.h"
#include "flags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Module create_module(const char *name, struct Tokenizer *tk)
{
    struct Module mod;
    strcpy(mod.name, name);

    mod.cp = 0;
    mod.cbuffer = BUFFER_SIZE;
    mod.code = malloc(BUFFER_SIZE);
    mod.table = create_table();
    mod.func_size = 0;
    mod.mod_size = 0;
    mod.tk = tk;
    return mod;
}

static struct SubModule create_sub_mod(const char *name)
{
    struct SubModule mod;
    strcpy(mod.name, name);
    mod.func_size = 0;
    return mod;
}

int find_mod_func_location(struct SubModule *sub, const char *func_name)
{
    int i;
    for (i = 0; i < sub->func_size; i++)
        if (!strcmp(sub->func_names[i], func_name))
            return i;

    strcpy(sub->func_names[sub->func_size], func_name);
    return sub->func_size++;
}

static void check_size(struct Module *mod, int size)
{
    if (mod->cp + size >= mod->cbuffer)
    {
        int times_over = (int)((mod->cp + size - mod->cbuffer) / BUFFER_SIZE) + 1;
        int new_size = mod->cbuffer + BUFFER_SIZE * times_over;
        LOG("Expanded memory from %i to %i\n", mod->cbuffer, new_size);

        mod->code = realloc(mod->code, new_size);
        mod->cbuffer += BUFFER_SIZE * times_over;
    }
}

void gen_char(struct Module *mod, char ins)
{
    check_size(mod, sizeof(char));
    mod->code[mod->cp++] = ins;
}

void gen_int(struct Module *mod, int i)
{
    check_size(mod, sizeof(int));
    memcpy(mod->code + mod->cp, &i, sizeof(int));
    mod->cp += sizeof(int);
}

void gen_string(struct Module *mod, const char *str)
{
    int size = strlen(str);
    check_size(mod, size + 1);
    mod->code[mod->cp] = size;
    strcpy(mod->code + mod->cp + 1, str);
    mod->cp += size + 1;
}

void delete_module(struct Module mod)
{
    free(mod.code);
}

static void parse_block(struct Module *mod, struct Tokenizer *tk);
static void assign_local(struct Module *mod, struct Node *node)
{
    struct Symbol symb;
    symb = lookup(&mod->table, node->left->s);
    if (symb.location == -1)
    {
        symb = create_symbol(&mod->table, node->left->s, 
            mod->loc++, NULL, 0);
    }

    compile_expression(mod, node->right);
    gen_char(mod, BC_ASSIGN_LOC);
    gen_int(mod, symb.location);
    delete_node(node);
}

static void parse_expression_statement(struct Module *mod, struct Tokenizer *tk)
{
    struct Node *node = parse_expression(tk);
    if (node->is_op && node->type == TK_ASSIGN)
    {
        if (node->left->type == TK_NAME)
        {
            assign_local(mod, node);
            return;
        }
    }
    compile_expression(mod, node);
    delete_node(node);
    gen_char(mod, BC_POP);
    gen_char(mod, 1);
}

static void parse_return(struct Module *mod, struct Tokenizer *tk)
{
    struct Node *node;
    match(tk, "return", TK_RETURN);

    node = parse_expression(tk);
    compile_expression(mod, node);
    delete_node(node);
    gen_char(mod, BC_RETURN);
}

static void parse_if(struct Module *mod, struct Tokenizer *tk)
{
    struct Node *node;
    int end_addr;

    match(tk, "if", TK_IF);
    node = parse_expression(tk);
    compile_expression(mod, node);
    delete_node(node);
    
    end_addr = mod->cp + 1;
    gen_char(mod, BC_JUMP_IF_NOT);
    gen_int(mod, 0);

    parse_block(mod, tk);
    memcpy(mod->code + end_addr, 
        &mod->cp, 4);
}

static void parse_statement(struct Module *mod, struct Tokenizer *tk)
{
    switch (tk->look.type)
    {
        case TK_RETURN: parse_return(mod, tk); break;
        case TK_IF: parse_if(mod, tk); break;
        default: parse_expression_statement(mod, tk); break;
    }
}

static void parse_block(struct Module *mod, struct Tokenizer *tk)
{
    if (tk->look.type == TK_OPEN_BLOCK)
    {
        match(tk, "{", TK_OPEN_BLOCK);
        while (tk->look.type != TK_CLOSE_BLOCK)
            parse_statement(mod, tk);
        match(tk, "}", TK_CLOSE_BLOCK);
    }
    else
        parse_statement(mod, tk);
}

static void parse_params(struct Module *mod, struct Tokenizer *tk)
{
    match(tk, "(", TK_OPEN_ARG);
    while (tk->look.type != TK_CLOSE_ARG)
    {
        struct Token param = match(tk, "Param", TK_NAME);
        if (tk->look.type != TK_CLOSE_ARG)
            match(tk, ",", TK_NEXT);
        
        LOG("Param '%s'\n", param.data);
        create_symbol(&mod->table, param.data, mod->param++, 
            NULL, SYMBOL_PARAMETER);
    }
    match(tk, ")", TK_CLOSE_ARG);
}

static struct Function *create_function(struct Module *mod, const char *name)
{
    create_symbol(&mod->table, name, mod->func_size, 
        NULL, SYMBOL_FUNCTION);

    struct Function *func = &mod->funcs[mod->func_size++];
    strcpy(func->name, name);
    func->start = mod->cp;
    return func;
}

void parse_function(struct Module *mod, struct Tokenizer *tk)
{
    struct Token name;
    struct Function *func;
    mod->loc = 0;
    mod->param = 0;

    match(tk, "func", TK_FUNC);
    name = match(tk, "Name", TK_NAME);
    func = create_function(mod, name.data);
    LOG("Function of name '%s'\n", name.data);
    
    push_scope(&mod->table);
    parse_params(mod, tk);
    parse_block(mod, tk);
    pop_scope(&mod->table);

    gen_char(mod, BC_PUSH_INT);
    gen_int(mod, 0);
    gen_char(mod, BC_RETURN);
    func->size = mod->loc;
    LOG("\n");
}

static void parse_import(struct Module *mod, struct Tokenizer *tk)
{
    struct Token name;
    match(tk, "import", TK_IMPORT);
    name = match(tk, "Name", TK_NAME);
    create_symbol(&mod->table, name.data, mod->mod_size, NULL, SYMBOL_MODULE);
    mod->mods[mod->mod_size++] = create_sub_mod(name.data);
}

static void parse_import_funcs(struct Module *mod, struct Tokenizer *tk)
{
    struct Token mod_name, func_name;
    struct SubModule sub;

    match(tk, "within", TK_WITHIN);
    mod_name = match(tk, "Name", TK_NAME);
    sub = create_sub_mod(mod_name.data);

    match(tk, "import", TK_IMPORT);
    for(;;)
    {
        func_name = match(tk, "Name", TK_NAME);
        strcpy(sub.func_names[sub.func_size++], func_name.data);
        if (tk->look.type != TK_NEXT)
            break;
        match(tk, ",", TK_NEXT);
    }
    mod->mods[mod->mod_size++] = sub;
}

static int calc_header_size(struct Module *mod)
{
    int size = 0, i, j;
    size += strlen(mod->name) + 1; // Mod name

    size += 4; // Func length
    for (i = 0; i < mod->func_size; i++)
    {
        struct Function func = mod->funcs[i];
        size += strlen(func.name) + 1; // Function name
        size += 4;                     // start location
    }

    size += 4; // Mod length
    for (i = 0; i < mod->mod_size; i++)
    {
        struct SubModule other = mod->mods[i];
        size += strlen(other.name) + 1; // Mod name
        for (j = 0; j < other.func_size; j++) // Func names
            size += strlen(other.func_names[i]) + 1;
    }

    return size;
}

#define WRITE_STRING(str, data, p) \
{ \
    int len = strlen(str); \
    data[p++] = len; \
    strcpy(data + p, str); \
    p += len; \
}

#define WRITE_INT(i, data, p) \
    memcpy(data + p, &i, 4); p += 4;

void write_header(struct Module *mod)
{
    int i, j, hp = 0;
    int header_size = calc_header_size(mod);
    mod->header = malloc(header_size);

    // Write mod name
    WRITE_STRING(mod->name, mod->header, hp);

    // Write function data
    WRITE_INT(mod->func_size, mod->header, hp);
    for (i = 0; i < mod->func_size; i++)
    {
        struct Function func = mod->funcs[i];
        WRITE_STRING(func.name, mod->header, hp);
        WRITE_INT(func.start, mod->header, hp);
        WRITE_INT(func.size, mod->header, hp);
    }

    // Write mod data
    WRITE_INT(mod->mod_size, mod->header, hp);
    for (i = 0; i < mod->mod_size; i++)
    {
        struct SubModule sub = mod->mods[i];
        WRITE_STRING(sub.name, mod->header, hp);

        // Mod functions
        WRITE_INT(sub.func_size, mod->header, hp);
        for (j = 0; j < sub.func_size; j++)
            WRITE_STRING(sub.func_names[j], mod->header, hp);
    }
}

void compile_module(struct Module *mod)
{
    struct Tokenizer *tk = mod->tk;
    
    // Compile functions
    while (tk->look.type != -1)
    {
        switch(tk->look.type)
        {
            case TK_IMPORT: parse_import(mod, tk); break;
            case TK_WITHIN: parse_import_funcs(mod, tk); break;
            case TK_FUNC: parse_function(mod, tk); break;
            default: F_ERROR(tk, "Unexpected token '%s'\n"); break;
        }
    }
    
    // Parse function headers
    write_header(mod);
}
