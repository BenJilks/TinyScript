#ifndef SYMBOL_H
#define SYMBOL_H

// Symbol flags
#define SYMBOL_FUNCTION     0b1000
#define SYMBOL_SYSTEM       0b0100
#define SYMBOL_PARAMETER    0b0010
#define SYMBOL_MODULE       0b0001

struct SymbolType;
struct Symbol
{
    char name[80];
    int location;
    struct SymbolType *type;
    int flags;
};

struct SymbolType
{
    struct Symbol attrs[80];
    int size;

    char name[80];
    int id;
};

struct SymbolTable
{
    struct Symbol symbs[80];
    int size;

    int scopes[80];
    int scope_pointer;

    struct SymbolType types[80];
    int type_size;
};

struct SymbolTable create_table();
struct SymbolType *create_type(struct SymbolTable *table, const char *name);

void create_symbol(struct SymbolTable *table, const char *name, 
    int location, struct SymbolType *type, int flags);
void create_atrr(struct SymbolType *table, const char *name, 
    int location, struct SymbolType *type, int flags);

struct Symbol lookup(struct SymbolTable *table, const char *name);
struct Symbol lookup_attr(struct SymbolType *table, const char *name);

void push_scope(struct SymbolTable *table);
void pop_scope(struct SymbolTable *table);

#endif // SYMBOL_H
