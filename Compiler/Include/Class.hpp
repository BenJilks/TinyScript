#pragma once
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include <vector>

class Function;
class Class
{
public:
    Class(string name, Tokenizer *tk, vector<char>* code, SymbolTable table);
    Symbol *FindAttribute(string name);
    Function *FindMethod(string name);
    int IndexOf(string name);
    void Compile();

    inline string Name() const { return name; }
    inline int Size() const { return attrs.size(); }
    inline void AddMethod(Function *func) { methods.push_back(func); }

private:
    void CompileAttr();
    void CompileMethod();

    string name;
    vector<Symbol*> attrs;
    vector<Function*> methods;
    vector<char>* code;
    SymbolTable table;
    Tokenizer *tk;

};
