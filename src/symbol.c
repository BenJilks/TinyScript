#include "symbol.h"
#include <string.h>
#include <stdlib.h>

struct SymbolTable create_table()
{
    struct SymbolTable table;
    table.size = 0;
    table.scope_pointer = 0;
    table.type_size = 0;
    table.symbs = malloc(sizeof(struct Symbol) * 80);
    table.types = malloc(sizeof(struct SymbolType) * 80);

    create_type(&table, "int", 0);
    create_type(&table, "float", 0);
    create_type(&table, "char", 0);
    create_type(&table, "bool", 0);
    return table;
}

struct SymbolType *create_type(struct SymbolTable *table, const char *name, int flags)
{
    struct SymbolType *type = &table->types[table->type_size++];
    strcpy(type->name, name);
    type->attrs = malloc(sizeof(struct Symbol) * 80);
    type->size = 0;
    type->id = table->type_size - 1;
    type->attr_size = 0;
    type->flags = flags;
    type->module = -1;
    return type;
}

void delete_symbol(struct SymbolTable *table, const char *name)
{
    int i;
    for (i = 0; i < table->size; i++)
        if (!strcmp(table->symbs[i].name, name))
            table->symbs[i].flags |= SYMBOL_DELETED;
}

void delete_table(struct SymbolTable table)
{
    int i;
    for (i = 0; i < table.type_size; i++)
        free(table.types[i].attrs);

    free(table.symbs);
    free(table.types);
}

struct Symbol *create_symbol(struct SymbolTable *table, const char *name, 
    int location, int flags)
{
    struct Symbol *symb = &table->symbs[table->size++];
    strcpy(symb->name, name);
    symb->location = location;
    symb->type = NULL;
    symb->flags = flags;
    return symb;
}

struct Symbol *create_atrr(struct SymbolType *table, const char *name, 
    int location, int flags)
{
    return create_symbol((struct SymbolTable *)table, name, location, flags);
}

struct Symbol lookup(struct SymbolTable *table, const char *name)
{
    int i;
    struct Symbol symb, null_s;
    
    // Search for the symbol
    for (i = 0; i < table->size; i++)
    {
        symb = table->symbs[i];
        if (!strcmp(symb.name, name) && !(symb.flags & SYMBOL_DELETED))
            return symb;
    }

    // If it could not find the symbol, 
    // return an empty one
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

struct SymbolType *lookup_type(struct SymbolTable *table, const char *name)
{
    int i;
    for (i = 0; i < table->type_size; i++)
        if (!strcmp(table->types[i].name, name))
            return &table->types[i];
    return NULL;
}

void push_scope(struct SymbolTable *table)
{
    table->scopes[table->scope_pointer++] = table->size;
}

void pop_scope(struct SymbolTable *table)
{
    table->size = table->scopes[--table->scope_pointer];
}
