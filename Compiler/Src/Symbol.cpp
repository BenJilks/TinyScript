#include "Symbol.hpp"
#include "Function.hpp"
#include "Class.hpp"
#include "Bytecode.hpp"
#include <algorithm>
#include <limits>

int SymbolTable::GetPrimType(string name)
{
    if (name == "int") return (int)Primitive::INT;
    if (name == "float") return (int)Primitive::FLOAT;
    if (name == "char") return (int)Primitive::CHAR;
    if (name == "bool") return (int)Primitive::BOOL;
    return (int)Primitive::OBJECT;
}

// Creates a new local symbol
void SymbolTable::AddLocal(string local) 
{
    Symbol *symb = new Symbol();
    symb->name = local;
    symb->is_const = false;
    symb->type = NULL;
    locals.push_back(symb);
}

// Create a new argument symbol
void SymbolTable::AddArgument(string arg) 
{ 
    Symbol *symb = new Symbol();
    symb->name = arg;
    symb->is_const = false;
    symb->type = NULL;
    args.push_back(symb);
}

// Finds the local location of a symbol by its name
int SymbolTable::FindLocation(string name)
{
    // Check for local table
    Symbol *symb = FindSymbol(name);
    int local_index = find(locals.begin(), locals.end(), symb) - locals.begin();
    if (local_index != locals.size())
        return local_index;
    
    // Check for arg table
    int arg_index = find(args.begin(), args.end(), symb) - args.begin();
    if (arg_index != args.size())
        // Args are backwards and negitive
        return -(args.size() - arg_index);
    
    // If it could not be found, then return max int value
    return numeric_limits<int>::max();
}

int SymbolTable::FindClassLocation(Class *c)
{
    int index = find(classes.begin(), classes.end(), c) - classes.begin();
    if (index != classes.size())
        return index;
    return -1;
}

// Finds a symbol by its name
Symbol *SymbolTable::FindSymbol(string name)
{
    // Search local
    for (Symbol *symb : locals)
        if (symb->name == name)
            return symb;
    
    // Search args
    for (Symbol *symb : args)
        if (symb->name == name)
            return symb;
    return NULL;
}

Function *SymbolTable::FindFunction(string name)
{
    for (Function *func : functions)
        if (func->Name() == name)
            return func;
    return NULL;
}

Class *SymbolTable::FindClass(string name)
{
    for (Class *c : classes)
        if (c->Name() == name)
            return c;
    return NULL;
}

void SymbolTable::PopScope()
{
    for (Symbol *symb : locals)
        delete symb;
    
    for (Symbol *symb : args)
        delete symb;
}

void SymbolTable::CleanUp()
{
    for (Function *func : functions)
        delete func;
    
    for (Class *c : classes)
        delete c;
}
