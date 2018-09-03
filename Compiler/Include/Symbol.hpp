#pragma once
#include <string>
#include <vector>
#include <tuple>
using namespace std;

class Scope;
class Symbol;
class SymbolType
{
public:
    SymbolType(string name, int id, Scope *attrs) :
        name(name), type_id(id), attrs(attrs) {}
    Symbol *Attr(string name);
    
    inline string Name() const { return name; }
    inline int TypeID() const { return type_id; }

private:
    string name;
    int type_id;
    Scope *attrs;
};

class Symbol
{
public:
    Symbol(string name, int location, bool is_parameter) :
        name(name), location(location), is_parameter(is_parameter), is_func(false), params(NULL), type(NULL) {}
    
    Symbol(string name, int location, bool is_sys, Scope *params) :
        name(name), location(location), is_sys(is_sys), is_func(true), params(params), type(NULL) {}

    inline void AssignType(SymbolType *t) { type = t; }

    inline string Name() const { return name; }
    inline int Location() const { return location; }
    inline SymbolType *Type() const { return type; }
    inline bool IsParameter() const { return is_parameter; }
    inline bool IsSystem() const { return is_sys; }
    inline bool IsFunction() const { return is_func; }
    
private:
    string name;
    int location;
    bool is_sys;
    bool is_parameter;
    bool is_func;
    Scope *params;
    SymbolType *type;

};

class GlobalScope;
class Scope
{
public:
    Scope(GlobalScope *global) :
        curr_loc(0), curr_param(0), max_size(0), global(global) {}
    
    inline GlobalScope *Global() const { return global; }
    inline int Length() const { return max_size; }
    Symbol *FindSymbol(string name);
    Symbol *MakeLocal(string name);
    Symbol *MakeFunc(string name, int location, bool is_sys, Scope *params);
    void MakeParameter(string name, SymbolType *type);

private:
    vector<Symbol> table;
    int curr_loc, curr_param;
    int max_size;
    GlobalScope *global;

};

class GlobalScope
{
public:
    GlobalScope();

    inline Scope *Globals() { return &globals; }
    void MakeType(string name, Scope *attrs);
    SymbolType *Type(string name);

private:
    vector<SymbolType> types;
    Scope globals;
    int prim_types;

};
