#pragma once
#include "Symbol.hpp"
#include "Tokenizer.hpp"

class Compiler
{
public:
    Compiler() {}
    
    bool Compile(string file_path);
    void DumpCode(string file_path);
    vector<char> GetDump();

    ~Compiler();
private:
    void DumpSysCalls(vector<char> &out_code);
    void DumpTypes(vector<char> &out_code);
    vector<char> Optimize();

    void CompileInclude(Tokenizer *tk);
    void CompileFunc(Tokenizer *tk);
    void CompileSysCall(Tokenizer *tk);
    void CompileClass(Tokenizer *tk);
    void CompileSysClass(Tokenizer *tk);

    vector<char> code;
    vector<string> compiled_files;
    vector<string> syscalls;
    SymbolTable table;
};
