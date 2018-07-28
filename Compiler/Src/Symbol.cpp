#include "Symbol.hpp"

Symbol *SymbolType::Attr(string name)
{
    return attrs->FindSymbol(name);
}

GlobalScope::GlobalScope() :
    globals(NULL)
{
    // Define primitive types
    types.push_back(SymbolType("int", -1, NULL));
    types.push_back(SymbolType("float", -1, NULL));
    types.push_back(SymbolType("char", -1, NULL));
    types.push_back(SymbolType("bool", -1, NULL));
    prim_types = types.size();
}

void GlobalScope::MakeType(string name, Scope *attrs)
{
    SymbolType type(name, types.size() - prim_types, attrs);
    types.push_back(type);
}

// Search for a type of a name
SymbolType *GlobalScope::Type(string name)
{
    for (int i = 0; i < types.size(); i++)
    {
        SymbolType *type = &types[i];
        if (type->Name() == name)
            return type;
    }
    return NULL;
}

Symbol *Scope::FindSymbol(string name)
{
    // Search in current scope
    for (int i = 0; i < table.size(); i++)
    {
        Symbol *symb = &table[i];
        if (symb->Name() == name)
            return symb;
    }

    // Search in global scope
    if (global != NULL)
    {
        Scope *globals = global->Globals();
        return globals->FindSymbol(name);
    }

    // If it could not be found, return error (null)
    return NULL;
}

Symbol *Scope::MakeLocal(string name)
{
    Symbol symb(name, curr_location++, false);
    table.push_back(symb);
    max_size++;
    return &table[table.size()-1];
}

Symbol *Scope::MakeFunc(string name, int location, bool is_sys, Scope *params)
{
    Symbol symb(name, location, is_sys, params);
    table.push_back(symb);
    return &table[table.size()-1];
}

void Scope::MakeParameter(string name, SymbolType *type)
{
    auto param = make_tuple(name, type);
    params.push_back(param);
}

void Scope::FinishParams()
{
    int curr_loc = -1;
    for (int i = params.size() - 1; i >= 0; i--)
    {
        auto param = params[i];
        Symbol symb(get<0>(param), curr_loc--, true);
        symb.AssignType(get<1>(param));
        table.push_back(symb);
    }
}
