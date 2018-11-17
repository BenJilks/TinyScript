#include "symbol.h"
#include <string.h>

struct SymbolTable create_table()
{
    struct SymbolTable table;
    table.size = 0;
    table.scope_pointer = 0;
    table.type_size = 0;
    return table;
}

struct SymbolType *create_type(struct SymbolTable *table, const char *name)
{
    struct SymbolType type;
    strcpy(type.name, name);
    type.size = 0;
    type.id = table->type_size;
    table->types[table->type_size] = type;
    return &table->types[table->type_size++];
}

void create_symbol(struct SymbolTable *table, const char *name, 
    int location, struct SymbolType *type, int flags)
{
    struct Symbol symb;
    strcpy(symb.name, name);
    symb.location = location;
    symb.type = type;
    symb.flags = flags;
    table->symbs[table->size++] = symb;
}

void create_atrr(struct SymbolType *table, const char *name, 
    int location, struct SymbolType *type, int flags)
{
    create_symbol((struct SymbolTable *)table, name, location, type, flags);
}

struct Symbol lookup(struct SymbolTable *table, const char *name)
{
    int i;
    for (i = 0; i < table->size; i++)
    {
        if (!strcmp(table->symbs[i].name, name))
            return table->symbs[i];
    }

    struct Symbol null_s;
    strcpy(null_s.name, name);
    null_s.location = -1;
    null_s.type = NULL;
    null_s.flags = 0;
    return null_s;
}

struct Symbol lookup_attr(struct SymbolType *table, const char *name)
{
    return lookup((struct SymbolTable *)table, name);
}

void push_scope(struct SymbolTable *table)
{
    table->scopes[table->scope_pointer++] = table->size;
}

void pop_scope(struct SymbolTable *table)
{
    table->size = table->scopes[--table->scope_pointer];
}
