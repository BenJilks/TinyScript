#pragma once
#include <string>
#include <vector>
using namespace std;

class Function;
class Class;

struct Symbol
{
    string name;
    Class *type;
    int prim_type;
    bool is_prim;
    bool is_const;
};

class SymbolTable
{
public:
    SymbolTable() {}
    static int GetPrimType(string name);
    
    inline void AddFunction(Function *func) { functions.push_back(func); }
    inline void AddClass(Class *c) { classes.push_back(c); }
    inline int Length() const { return locals.size(); }
    void AddLocal(string local);
    void AddArgument(string arg);

    int FindLocation(string name);
    Symbol *FindSymbol(string name);
    Function *FindFunction(string name);
    Class *FindClass(string name);

private:
    vector<Symbol*> locals;
    vector<Symbol*> args;
    vector<Function*> functions;
    vector<Class*> classes;

};
