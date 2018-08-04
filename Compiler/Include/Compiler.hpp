#pragma once
#include "Symbol.hpp"
#include "Tokenizer.hpp"
#include "CodeGen.hpp"

using External = tuple<string, vector<string>>;

class Class;
class Compiler
{
public:
    Compiler(): 
        pointer(0) {}
    
    bool Compile(string file_path);
    void DumpCode(string file_path);
    vector<char> GetDump();

    ~Compiler();
private:
    void DumpExternals(CodeGen &out_code);
    void DumpSysCalls(CodeGen &out_code);
    void DumpTypes(CodeGen &out_code);
    CodeGen Optimize();

    void CompileInclude(Tokenizer *tk);
    void CompileExternal(Tokenizer *tk);
    void CompileExternalCall(Tokenizer *tk, vector<string> &calls);
    void CompileExternalClass(Tokenizer *tk, vector<string> &calls);
    void CompileFunc(Tokenizer *tk);
    void CompileClass(Tokenizer *tk);

    CodeGen code;
    vector<string> compiled_files;
    vector<string> syscalls;
    vector<External> externals;
    int pointer;

    vector<Class*> classes;
    GlobalScope global_scope;
    Scope *globals;
};
