#ifndef SYMBOL_H
#define SYMBOL_H

// Symbol flags
#define SYMBOL_FUNCTION     0b1000000
#define SYMBOL_SYSTEM       0b0100000
#define SYMBOL_PARAMETER    0b0010000
#define SYMBOL_MODULE       0b0001000
#define SYMBOL_EXTERNAL     0b0000100
#define SYMBOL_UNKOWN       0b0000010
#define SYMBOL_DELETED      0b0000001

// Type flags
#define TYPE_EXTERNAL       0b1

struct SymbolType;
struct Symbol
{
    char name[80];
    int location, module;
    struct SymbolType *type;
    int flags;
};

struct SymbolType
{
    struct Symbol *attrs;
    int size;
    
    int id, module;
    char name[80];
    int attr_size, method_size;
    int flags;
};

struct SymbolTable
{
    struct Symbol *symbs;
    int size;

    int scopes[80];
    int scope_pointer;

    struct SymbolType *types;
    int type_size;
};

struct SymbolTable create_table();
struct SymbolType *create_type(struct SymbolTable *table, const char *name, int flags);
void delete_symbol(struct SymbolTable *table, const char *name);
void delete_table(struct SymbolTable table);

struct Symbol *create_symbol(struct SymbolTable *table, const char *name, 
    int location, int flags);
struct Symbol *create_atrr(struct SymbolType *table, const char *name, 
    int location, int flags);

struct Symbol lookup(struct SymbolTable *table, const char *name);
struct Symbol lookup_attr(struct SymbolType *table, const char *name);
struct SymbolType *lookup_type(struct SymbolTable *table, const char *name);

void push_scope(struct SymbolTable *table);
void pop_scope(struct SymbolTable *table);

#endif // SYMBOL_H
