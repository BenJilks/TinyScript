#include "expression.h"
#include "bytecode.h"
#include "flags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char is_add_op(int op)
{
    return op == TK_ADD || op == TK_SUB;
}

static char is_mul_op(int op)
{
    return op == TK_MUL || op == TK_DIV;
}

static char is_object_op(int op)
{
    return op == TK_IN;
}

static char is_assign_op(int op)
{
    return op == TK_ASSIGN;
}

static struct Node *parse_operation(struct Node *left, int op, struct Node *right)
{
    struct Node *node = malloc(sizeof(struct Node));
    node->left = left;
    node->right = right;
    node->type = op;
    node->is_op = 1;
    return node;
}

void parse_name(struct Tokenizer *tk, struct Node *node, const char *data)
{
    struct Token func_name;

    strcpy(node->s, data);
    if (tk->look.type == TK_OF)
    {
        match(tk, ":", TK_OF);
        func_name = match(tk, "Name", TK_NAME);
        strcpy(node->mod_func, func_name.data);
    }

    if (tk->look.type == TK_OPEN_ARG)
    {
        match(tk, "(", TK_OPEN_ARG);
        while (tk->look.type != TK_CLOSE_ARG)
        {
            node->args[node->arg_size++] = parse_expression(tk);
            if (tk->look.type != TK_NEXT)
                break;
            match(tk, ",", TK_NEXT);
        }
        match(tk, ")", TK_CLOSE_ARG);
    }
}

struct Node *parse_data(struct Tokenizer *tk)
{
    struct Node *node = malloc(sizeof(struct Node));
    struct Token token = tk->look;
    node->type = token.type;
    node->is_op = 0;
    node->arg_size = 0;
    tk->look = next(tk);
    
    char *data = token.data;
    switch (node->type)
    {
        case TK_INT: node->i = atoi(data); break;
        case TK_FLOAT: node->f = atof(data); break;
        case TK_CHAR: node->c = data[0]; break;
        case TK_BOOL: node->b = strcmp(data, "true") ? 0 : 1; break;
        case TK_STRING: strcpy(node->s, data); break;
        case TK_NAME: parse_name(tk, node, data); break;
        default: printf("Error: Unexpected token '%s', expected expression\n", data);
    }

    return node;
}

#define PARSE_OPERATION(check, term) \
    struct Node *left = term(tk); \
    while (check(tk->look.type)) \
    { \
        int op = tk->look.type; \
        tk->look = next(tk); \
         \
        struct Node *right = term(tk); \
        left = parse_operation(left, op, right); \
    } \
    return left;

#define PARSE_FUNCTION(name, check, term) \
    static struct Node *name(struct Tokenizer *tk) \
    { \
        PARSE_OPERATION(check, term); \
    }

PARSE_FUNCTION(parse_term, is_object_op, parse_data);
PARSE_FUNCTION(parse_factor, is_mul_op, parse_term);
PARSE_FUNCTION(parse_statement, is_add_op, parse_factor);

struct Node *parse_expression(struct Tokenizer *tk)
{
    PARSE_OPERATION(is_assign_op, parse_statement);
}

static void do_operation(struct Module *mod, int op)
{
    switch (op)
    {
        case TK_ADD: gen_char(mod, BC_ADD); break;
        case TK_SUB: gen_char(mod, BC_SUB); break;
        case TK_MUL: gen_char(mod, BC_MUL); break;
        case TK_DIV: gen_char(mod, BC_DIV); break;
        case TK_ASSIGN: gen_char(mod, BC_ASSIGN); break;
        default: printf("op not imp\n"); break;
    }
}

static void call_function(struct Module *mod, struct Node *node, struct Symbol symb)
{
    LOG("call function '%s' at %i with %i args\n", 
            symb.name, symb.location, node->arg_size);
    
    int i;
    for (i = 0; i < node->arg_size; i++)
        compile_expression(mod, node->args[i]);
    gen_char(mod, BC_CALL);
    gen_int(mod, node->arg_size);
    gen_int(mod, symb.location);
}

static void compile_name(struct Module *mod, struct Node *node, char addr_mode)
{
    const char *name = node->s;
    struct Symbol symb = lookup(&mod->table, name);
    if (symb.location == -1)
    {
        F_ERROR(mod->tk, "Could not find symbol '%s'", name);
        return;
    }

    // If the symbol is a funtion
    if (symb.flags & SYMBOL_FUNCTION)
    {
        call_function(mod, node, symb);
        return;
    }

    // If the symbol is a module
    if (symb.flags & SYMBOL_MODULE)
    {
        struct SubModule *sub;
        int loc, i;
        for (i = 0; i < node->arg_size; i++)
            compile_expression(mod, node->args[i]);

        sub = &mod->mods[symb.location];
        loc = find_mod_func_location(sub, node->mod_func);
        gen_char(mod, BC_CALL_MOD);
        gen_int(mod, node->arg_size);
        gen_int(mod, symb.location);
        gen_int(mod, loc);
        return;
    }

    if (addr_mode)
    {
        gen_char(mod, BC_PUSH_INT);
        if (symb.flags & SYMBOL_PARAMETER)
            gen_int(mod, -symb.location - 1);
        else
            gen_int(mod, symb.location);
        LOG("push location of '%s' at %i\n", name, symb.location);
    }
    else
    {
        char ins = (symb.flags & SYMBOL_PARAMETER) ? BC_PUSH_ARG : BC_PUSH_LOC;
        gen_char(mod, ins);
        gen_int(mod, symb.location);
        LOG("push value of '%s' at %i\n", name, symb.location);
    }
}

static void compile_term(struct Module *mod, struct Node *node, char addr_mode)
{
    switch (node->type)
    {
        case TK_INT: gen_char(mod, BC_PUSH_INT); gen_int(mod, node->i); break;
        case TK_FLOAT: gen_char(mod, BC_PUSH_FLOAT); gen_int(mod, node->i); break;
        case TK_CHAR: gen_char(mod, BC_PUSH_CHAR); gen_char(mod, node->c); break;
        case TK_BOOL: gen_char(mod, BC_PUSH_BOOL); gen_char(mod, node->b); break;
        case TK_STRING: gen_char(mod, BC_PUSH_STRING); gen_string(mod, node->s); break;
        case TK_NAME: compile_name(mod, node, addr_mode); break;
    }
}

void compile_assign_op(struct Module *mod, struct Node *node, char addr_mode)
{
    if (node->left->type == TK_NAME)
    {
        const char *name = node->left->s;
        if (lookup(&mod->table, name).location == -1)
            create_symbol(&mod->table, name, mod->loc++, NULL, 0);
    }

    compile_node(mod, node->left, 1);
    compile_node(mod, node->right, addr_mode);
    do_operation(mod, node->type);
}

void compile_node(struct Module *mod, struct Node *node, char addr_mode)
{
    if (node->is_op)
    {
        if (node->type == TK_ASSIGN)
        {
            compile_assign_op(mod, node, addr_mode);
            return;
        }

        compile_node(mod, node->left, addr_mode);
        compile_node(mod, node->right, addr_mode);
        do_operation(mod, node->type);
        return;
    }

    compile_term(mod, node, addr_mode);
}

void compile_expression(struct Module *mod, struct Node *node)
{
    compile_node(mod, node, 0);
}

void delete_node(struct Node *node)
{
    if (node->is_op)
    {
        delete_node(node->left);
        delete_node(node->right);
    }
    free(node);
}
