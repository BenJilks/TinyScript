#pragma once
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include "CodeGen.hpp"
#include <vector>

class Function;
class Class
{
public:
    Class(string name, Tokenizer *tk, CodeGen *code, GlobalScope *global);
    void CompileSys(vector<string> &syscalls, int *pointer);
    void Compile();

    inline Scope *Attrs() const { return attrs; }
    inline string Name() const { return name; }
    inline int Size() const { return attrs->Length(); }
    inline bool IsSysClass() const { return is_sys_class; }

    ~Class();

private:
    void CompileAttr();
    void CompileMethod();

    string name;
    bool is_sys_class;
    CodeGen *code;
    Scope *attrs;
    GlobalScope *global;
    Tokenizer *tk;

};
