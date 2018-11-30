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
    mod.label_counter = 0;
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

// Allocate a new label id
int create_label(struct Module *mod)
{
    int id = mod->label_counter++;
    mod->labels[id].use_count = 0;
    mod->labels[id].addr = -1;
    return id;
}

// Reserve a place for the address to be placed and mark it
void gen_label(struct Module *mod, int id)
{
    struct Label *label;
    label = &mod->labels[id];
    label->uses[label->use_count++] = mod->cp;
    gen_int(mod, 0);
}

// Assign the address of a label
void assign_label(struct Module *mod, int id)
{
    struct Label *label;
    label = &mod->labels[id];
    label->addr = mod->cp;
}

// Gen the code for all currently used labels
void finish_labels(struct Module *mod)
{
    struct Label label;
    int i, j, addr;

    for (i = 0; i < mod->label_counter; i++)
    {
        label = mod->labels[i];
        addr = label.addr - mod->start_addr;

        for (j = 0; j < label.use_count; j++)
            memcpy(mod->code + label.uses[j], 
                &addr, sizeof(int));
    }
    mod->label_counter = 0;
}

void delete_module(struct Module mod)
{
    free(mod.code);
}

static void parse_block(struct Module *mod, struct Tokenizer *tk, int l_start, int l_end);
static struct Symbol assign_local(struct Module *mod, struct Node *node)
{
    struct Symbol symb;
    symb = lookup(&mod->table, node->left->s);
    if (symb.location == -1)
    {
        symb = *create_symbol(&mod->table, node->left->s, 
            mod->loc++, 0);
    }

    compile_expression(mod, node->right);
    gen_char(mod, BC_ASSIGN_LOC);
    gen_int(mod, symb.location);
    delete_node(node);
    return symb;
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

static void parse_push(struct Module *mod, struct Tokenizer *tk)
{
    struct Node *node;

    node = parse_expression(tk);
    compile_expression(mod, node);
    delete_node(node);
}

static void parse_if(struct Module *mod, struct Tokenizer *tk, int ll_start, int ll_end)
{
    int l_last, else_type;
    int l_next = create_label(mod);
    int l_end = create_label(mod);

    // Parse and gen if statement header
    match(tk, "if", TK_IF);
    parse_push(mod, tk);
    gen_char(mod, BC_JUMP_IF_NOT);
    gen_label(mod, l_next);
    parse_block(mod, tk, ll_start, ll_end);

    // While there is anouther else of elif statement
    else_type = tk->look.type;
    while (else_type == TK_ELSE_IF || else_type == TK_ELSE)
    {
        // Finish last statement, jump out of if
        l_last = l_next;
        l_next = create_label(mod);
        gen_char(mod, BC_JUMP);
        gen_label(mod, l_end);

        // Jump to here
        assign_label(mod, l_last);
        tk->look = next(tk);
        if (else_type == TK_ELSE_IF)
        {
            // If elif, jump to next if condition not met
            parse_push(mod, tk);
            gen_char(mod, BC_JUMP_IF_NOT);
            gen_label(mod, l_next);
        }
        
        // Parse else block
        parse_block(mod, tk, ll_start, ll_end);
        else_type = tk->look.type;
    }

    // Set the next jump and ending labels to 
    // the end of the statement
    assign_label(mod, l_end);
    assign_label(mod, l_next);
}

static void parse_while(struct Module *mod, struct Tokenizer *tk)
{
    int l_start = create_label(mod);
    int l_end = create_label(mod);
    assign_label(mod, l_start);

    match(tk, "while", TK_WHILE);
    parse_push(mod, tk);
    gen_char(mod, BC_JUMP_IF_NOT);
    gen_label(mod, l_end);

    parse_block(mod, tk, l_start, l_end);
    gen_char(mod, BC_JUMP);
    gen_label(mod, l_start);

    assign_label(mod, l_end);
}

static void parse_for(struct Module *mod, struct Tokenizer *tk)
{
    int l_start = create_label(mod);
    int l_end = create_label(mod);
    struct Node *node;
    struct Symbol symb;

    // Compile assign of local
    match(tk, "for", TK_FOR);
    node = parse_expression(tk);
    symb = assign_local(mod, node);

    // Jump to end if not parsed max value
    assign_label(mod, l_start);
    match(tk, "to", TK_TO);
    gen_char(mod, BC_PUSH_LOC);
    gen_int(mod, symb.location);
    parse_push(mod, tk);
    gen_char(mod, BC_LESS_THAN);
    gen_char(mod, BC_JUMP_IF_NOT);
    gen_label(mod, l_end);

    // Increase local by one and jump to start
    parse_block(mod, tk, l_start, l_end);
    gen_char(mod, BC_INC_LOC);
    gen_int(mod, symb.location);
    gen_int(mod, 1);
    gen_char(mod, BC_JUMP);
    gen_label(mod, l_start);
    assign_label(mod, l_end);
}

static void parse_break(struct Module *mod, struct Tokenizer *tk, int l_start, int l_end)
{
    match(tk, "break", TK_BREAK);
    gen_char(mod, BC_JUMP);
    gen_label(mod, l_end);
}

static void parse_continue(struct Module *mod, struct Tokenizer *tk, int l_start, int l_end)
{
    match(tk, "continue", TK_CONTINUE);
    gen_char(mod, BC_JUMP);
    gen_label(mod, l_start);
}

static void parse_statement(struct Module *mod, struct Tokenizer *tk, int l_start, int l_end)
{
    switch (tk->look.type)
    {
        case TK_RETURN: parse_return(mod, tk); break;
        case TK_IF: parse_if(mod, tk, l_start, l_end); break;
        case TK_WHILE: parse_while(mod, tk); break;
        case TK_FOR: parse_for(mod, tk); break;
        case TK_BREAK: parse_break(mod, tk, l_start, l_end); break;
        case TK_CONTINUE: parse_continue(mod, tk, l_start, l_end); break;
        default: parse_expression_statement(mod, tk); break;
    }
}

static void parse_block(struct Module *mod, struct Tokenizer *tk, int l_start, int l_end)
{
    if (tk->look.type == TK_OPEN_BLOCK)
    {
        match(tk, "{", TK_OPEN_BLOCK);
        while (tk->look.type != TK_CLOSE_BLOCK)
            parse_statement(mod, tk, l_start, l_end);
        match(tk, "}", TK_CLOSE_BLOCK);
    }
    else
        parse_statement(mod, tk, l_start, l_end);
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
        create_symbol(&mod->table, param.data, mod->param++, SYMBOL_PARAMETER);
    }
    match(tk, ")", TK_CLOSE_ARG);
}

static struct Function *create_function(struct Module *mod, const char *name)
{
    struct Function *func = &mod->funcs[mod->func_size++];
    strcpy(func->name, name);
    func->start = mod->cp;
    return func;
}

void start_new_function(struct Module *mod)
{
    mod->loc = 0;
    mod->param = 0;
    mod->start_addr = mod->cp;
}

void parse_function_body(struct Module *mod, struct Tokenizer *tk)
{
    push_scope(&mod->table);
    parse_params(mod, tk);
    parse_block(mod, tk, -1, -1);
    pop_scope(&mod->table);

    gen_char(mod, BC_PUSH_INT);
    gen_int(mod, 0);
    gen_char(mod, BC_RETURN);
}

void parse_function(struct Module *mod, struct Tokenizer *tk)
{
    struct Token name;
    struct Function *func;
    start_new_function(mod);

    match(tk, "func", TK_FUNC);
    name = match(tk, "Name", TK_NAME);
    func = create_function(mod, name.data);
    create_symbol(&mod->table, name.data, mod->func_size-1, SYMBOL_FUNCTION);
    LOG("Function of name '%s'\n", name.data);
    
    parse_function_body(mod, tk);
    func->size = mod->loc;
    finish_labels(mod);
    LOG("\n");
}

static void parse_attr(struct SymbolType *type, struct Tokenizer *tk)
{
    struct Token attr;

    attr = match(tk, "Name", TK_NAME);
    create_atrr(type, attr.data, type->attr_size++, 0);
    LOG("Attr '%s'\n", attr.data);
}

static void parse_method(struct SymbolType *type, struct Module *mod, struct Tokenizer *tk)
{
    struct Token name;
    struct Function *func;
    start_new_function(mod);

    match(tk, "func", TK_FUNC);
    name = match(tk, "Name", TK_NAME);
    func = create_function(mod, name.data);
    create_atrr(type, name.data, mod->func_size-1, SYMBOL_FUNCTION);
    LOG("Method of name '%s'\n", name.data);

    parse_function_body(mod, tk);
    func->size = mod->loc;
    finish_labels(mod);
    LOG("\n");
}

static void parse_class(struct Module *mod, struct Tokenizer *tk)
{
    struct Token name;
    struct SymbolType *type;
    
    match(tk, "class", TK_CLASS);
    name = match(tk, "Name", TK_NAME);
    type = create_type(&mod->table, name.data);
    LOG("Class named '%s'\n", name.data);

    match(tk, "{", TK_OPEN_BLOCK);
    while (tk->look.type != TK_CLOSE_BLOCK)
    {
        switch(tk->look.type)
        {
            case TK_NAME: parse_attr(type, tk); break;
            case TK_FUNC: parse_method(type, mod, tk); break;
            default: F_ERROR(tk, "Unexpected token '%s' in class '%s' body", 
                tk->look.data, name.data); tk->look = next(tk); break;
        }
    }
    match(tk, "}", TK_CLOSE_BLOCK);
}

static void parse_import(struct Module *mod, struct Tokenizer *tk)
{
    struct Token name;
    match(tk, "import", TK_IMPORT);
    name = match(tk, "Name", TK_NAME);
    create_symbol(&mod->table, name.data, mod->mod_size, SYMBOL_MODULE);
    mod->mods[mod->mod_size++] = create_sub_mod(name.data);
}

static void parse_import_funcs(struct Module *mod, struct Tokenizer *tk)
{
    struct Token mod_name, func_name;
    struct SubModule sub;
    struct Symbol *symb;

    match(tk, "within", TK_WITHIN);
    mod_name = match(tk, "Name", TK_NAME);
    sub = create_sub_mod(mod_name.data);

    match(tk, "import", TK_IMPORT);
    for(;;)
    {
        func_name = match(tk, "Name", TK_NAME);
        symb = create_symbol(&mod->table, func_name.data, 
            sub.func_size, SYMBOL_FUNCTION | SYMBOL_MOD_FUNC);
        symb->location = mod->mod_size;

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
        size += 8;                     // Start location and size
    }

    size += 4; // Type length
    for (i = 0; i < mod->table.type_size; i++)
    {
        struct SymbolType type = mod->table.types[i];
        size += strlen(type.name) + 1; // Type name
        size += 4;                     // Type size
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

    // Write type data
    WRITE_INT(mod->table.type_size, mod->header, hp);
    for (i = 0; i < mod->table.type_size; i++)
    {
        struct SymbolType type = mod->table.types[i];
        WRITE_STRING(type.name, mod->header, hp);
        WRITE_INT(type.size, mod->header, hp);
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
            case TK_CLASS: parse_class(mod, tk); break;
            default: F_ERROR(tk, "Unexpected token '%s'\n"); break;
        }
    }
    
    // Parse function headers
    write_header(mod);
}
