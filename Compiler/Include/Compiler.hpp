#pragma once
#include "Symbol.hpp"
#include "Tokenizer.hpp"
#include "CodeGen.hpp" 

using External = tuple<string, vector<string>>;

class Function;
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
    void DumpTypes(CodeGen &out_code);
    void DumpFunctions(CodeGen &out_code);

    void CompileInclude(Tokenizer *tk);
    void CompileExternal(Tokenizer *tk);
    void CompileExternalCall(Tokenizer *tk, vector<string> &calls);
    void CompileExternalClass(Tokenizer *tk, vector<string> &calls);
    void CompileFunc(Tokenizer *tk);
    void CompileClass(Tokenizer *tk);

    vector<Function> functions;
    vector<string> compiled_files;
    vector<External> externals;
    int pointer;

    vector<Class*> classes;
    GlobalScope global_scope;
    Scope *globals;
};
