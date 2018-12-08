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

static char is_logic_op(int op)
{
    return op == TK_MORE_THAN || op == TK_LESS_THAN || 
        op == TK_EQUAL;
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
        node->has_from = 1;
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
    node->has_from = 0;
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
        case TK_OPEN_ARG: 
            free(node); node = parse_expression(tk); 
            match(tk, ")", TK_CLOSE_ARG); 
            break;
        default: F_ERROR(tk, "Unexpected token '%s', expected expression", data);
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
PARSE_FUNCTION(parse_logic, is_add_op, parse_factor);
PARSE_FUNCTION(parse_statement, is_logic_op, parse_logic);

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
        case TK_MORE_THAN: gen_char(mod, BC_MORE_THAN); break;
        case TK_LESS_THAN: gen_char(mod, BC_LESS_THAN); break;
        case TK_EQUAL: gen_char(mod, BC_EQUAL_TO); break;
        case TK_ASSIGN: gen_char(mod, BC_ASSIGN); break;
        default: printf("Error: op not imp\n"); break;
    }
}

static struct SymbolType *call_function(struct Module *mod, struct Node *node, struct Symbol symb, int extra)
{
    LOG("call function '%s' at %i with %i args\n", 
            symb.name, symb.location, node->arg_size);
    
    int i;
    for (i = 0; i < node->arg_size; i++)
        compile_expression(mod, node->args[i]);
    
    if (symb.flags & SYMBOL_MOD_FUNC)
    {
        gen_char(mod, BC_CALL_MOD);
        gen_int(mod, node->arg_size + extra);
        gen_int(mod, symb.module);
        gen_int(mod, symb.location);
    }
    else
    {
        gen_char(mod, BC_CALL);
        gen_int(mod, node->arg_size + extra);
        gen_int(mod, symb.location);
    }

    return symb.type;
}

static struct SymbolType *compile_create_object(struct Module *mod, struct Node *node, struct SymbolType *type)
{
    struct Symbol constructor;
    gen_char(mod, BC_CREATE_OBJECT);
    gen_int(mod, type->id);

    constructor = lookup_attr(type, type->name);
    if (constructor.location != -1)
    {
        gen_char(mod, BC_DUP_LAST);
        call_function(mod, node, constructor, 1);
        gen_char(mod, BC_POP);
        gen_char(mod, 1);
    }
    return type;
}

static struct SymbolType *compile_name(struct Module *mod, struct Node *node, char addr_mode)
{
    const char *name;
    struct Symbol symb;
    struct SymbolType *type;

    name = node->s;
    symb = lookup(&mod->table, name);
    type = lookup_type(&mod->table, name);
    if (symb.location == -1)
    {
        if (type != NULL)
            return compile_create_object(mod, node, type);
        
        F_ERROR(mod->tk, "Could not find symbol '%s'", name);
        return NULL;
    }

    // If the symbol is a funtion
    if (symb.flags & SYMBOL_FUNCTION)
        return call_function(mod, node, symb, 0);

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
        return symb.type;
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

    return symb.type;
}

static struct SymbolType *compile_term(struct Module *mod, struct Node *node, char addr_mode)
{
    switch (node->type)
    {
        case TK_INT: gen_char(mod, BC_PUSH_INT); gen_int(mod, node->i); break;
        case TK_FLOAT: gen_char(mod, BC_PUSH_FLOAT); gen_int(mod, node->i); break;
        case TK_CHAR: gen_char(mod, BC_PUSH_CHAR); gen_char(mod, node->c); break;
        case TK_BOOL: gen_char(mod, BC_PUSH_BOOL); gen_char(mod, node->b); break;
        case TK_STRING: gen_char(mod, BC_PUSH_STRING); gen_string(mod, node->s); break;
        case TK_NAME: return compile_name(mod, node, addr_mode);
    }
    return NULL;
}

static void check_var_exists(struct Module *mod, struct Node *node)
{
    struct Symbol *symb;
    const char *name, *type_name;

    if (node->type == TK_NAME)
    {
        name = node->s;
        type_name = node->mod_func;
        if (lookup(&mod->table, name).location == -1)
        {
            // Create new local var
            symb = create_symbol(&mod->table, name, mod->loc++, 0);

            // Set static type, if exists
            if (node->has_from)
            {
                symb->type = lookup_type(&mod->table, type_name);
                LOG("Assigned static type '%s'\n", type_name);
                if (symb->type == NULL)
                    printf("Error: Type '%s' could not be found\n", type_name);
            }
        }
    }
}

static struct Symbol find_attr(struct Module *mod, struct SymbolType *stype, struct Node *node)
{
    char *type_name = node->s;
    char *attr_name = node->mod_func;
    struct SymbolType *type;
    struct Symbol attr;
    attr.location = -1;

    // Find type
    if (node->has_from)
    {
        type = lookup_type(&mod->table, type_name);
        if (type == NULL)
        {
            printf("Error: Unknown type '%s'\n", type_name);
            return attr;
        }
    }
    else
    {
        type = stype;
        attr_name = type_name;
        type_name = type->name;
        if (type == NULL)
        {
            printf("Error: Symbol does not have a static type");
            return attr;
        }
    }

    // Find attr in type
    attr = lookup_attr(type, attr_name);
    if (attr.location == -1)
    {
        printf("Error: No member '%s' in type '%s'\n", 
            attr_name, type_name);
        return attr;
    }

    return attr;
}

static struct SymbolType *compile_assign_op(struct Module *mod, struct Node *node, char addr_mode)
{
    struct Symbol attr;
    struct SymbolType *type;
    check_var_exists(mod, node->left);

    if (node->left->type == TK_IN)
    {
        type = compile_node(mod, node->left->left, addr_mode);
        attr = find_attr(mod, type, node->left->right);
        compile_node(mod, node->right, addr_mode);
        gen_char(mod, BC_ASSIGN_ATTR);
        gen_int(mod, attr.location);
        return NULL;
    }
    
    compile_node(mod, node->left, 1);
    compile_node(mod, node->right, addr_mode);
    do_operation(mod, node->type);
    return NULL;
}

static struct SymbolType *compile_in_op(struct Module *mod, struct Node *node, char addr_mode)
{
    struct Symbol attr;
    struct SymbolType *type;

    type = compile_node(mod, node->left, addr_mode);
    attr = find_attr(mod, type, node->right);
    if (attr.flags & SYMBOL_FUNCTION)
    {
        call_function(mod, node->right, attr, 1);
    }
    else
    {
        gen_char(mod, BC_PUSH_ATTR);
        gen_int(mod, attr.location);
    }
    return attr.type;
}

static struct SymbolType *compile_op(struct Module *mod, struct Node *node, char addr_mode)
{
    switch(node->type)
    {
        case TK_ASSIGN: return compile_assign_op(mod, node, addr_mode);
        case TK_IN: return compile_in_op(mod, node, addr_mode);
        
        default:
            compile_node(mod, node->left, addr_mode);
            compile_node(mod, node->right, addr_mode);
            do_operation(mod, node->type);
            return NULL;
    }
}

struct SymbolType *compile_node(struct Module *mod, struct Node *node, char addr_mode)
{
    if (node->is_op)
        return compile_op(mod, node, addr_mode);
    else
        return compile_term(mod, node, addr_mode);
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
