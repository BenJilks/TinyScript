#pragma once
#include "Symbol.hpp"
#include "Tokenizer.hpp"
#include "CodeGen.hpp"

class Class;
class Compiler
{
public:
    Compiler() {}
    
    bool Compile(string file_path);
    void DumpCode(string file_path);
    vector<char> GetDump();

    ~Compiler();
private:
    void DumpSysCalls(CodeGen &out_code);
    void DumpTypes(CodeGen &out_code);
    CodeGen Optimize();

    void CompileInclude(Tokenizer *tk);
    void CompileFunc(Tokenizer *tk);
    void CompileSysCall(Tokenizer *tk);
    void CompileClass(Tokenizer *tk);
    void CompileSysClass(Tokenizer *tk);

    CodeGen code;
    vector<string> compiled_files;
    vector<string> syscalls;

    vector<Class*> classes;
    GlobalScope global_scope;
    Scope *globals;
};
