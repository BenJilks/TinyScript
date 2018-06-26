#include "Compiler.hpp"
#include "Class.hpp"
#include "Function.hpp"
#include "Bytecode.hpp"
#include <iostream>
#include <algorithm>

bool Compiler::Compile(string file_path)
{
    Tokenizer tk(file_path);
    while (tk.LookType() != TkType::EndOfFile && !tk.HasError())
    {
        switch(tk.LookType())
        {
            case TkType::Include: CompileInclude(&tk); break;
            case TkType::Function: CompileFunc(&tk); break;
            case TkType::SysCall: CompileSysCall(&tk); break;
            case TkType::Class: CompileClass(&tk); break;
            case TkType::SysClass: CompileSysClass(&tk); break;
            default: tk.Error("Unexpect token '" + tk.LookData() + "' in global scope"); break;
        }
    }

    return tk.HasError();
}

void Compiler::DumpCode(string file_path)
{
    FILE *file = fopen(file_path.c_str(), "wb");
    vector<char> out_code = GetDump();
    fwrite(&out_code[0], sizeof(char), out_code.size(), file);
    fclose(file);
}

template<typename T>
static void PushData(T data, vector<char> &code)
{
    char *bytes = (char*)&data;
    code.insert(code.end(), bytes, 
        bytes + sizeof(T));
}

static void DumpString(string str, vector<char> &out_code)
{
    char length = str.length();
    out_code.push_back(length);
    out_code.insert(out_code.end(), 
        str.begin(), str.end());
}

void Compiler::DumpSysCalls(vector<char> &out_code)
{
    char count = syscalls.size();
    out_code.push_back(count);

    for (string syscall : syscalls)
        DumpString(syscall, out_code);
}

static void DumpOperator(Class *c, string name, vector<char> &out_code)
{
    Function *func = c->FindMethod(name);
    if (func == NULL)
        PushData(-1, out_code);
    else
        PushData(func->Location(), out_code);
}

void Compiler::DumpTypes(vector<char> &out_code)
{
    char count = table.Classes().size();
    out_code.push_back(count);

    // Push class data:
    // string: name, int: size
    for (Class *c : table.Classes())
    {
        DumpString(c->Name(), out_code);
        PushData(c->Size(), out_code);
        PushData(c->IsSysClass(), out_code);
        DumpOperator(c, "operator_add", out_code);
        DumpOperator(c, "operator_subtract", out_code);
        DumpOperator(c, "operator_multiply", out_code);
        DumpOperator(c, "operator_divide", out_code);
        DumpOperator(c, "operator_to_string", out_code);
        DumpOperator(c, "operator_get_index", out_code);
        DumpOperator(c, "operator_set_index", out_code);
    }
}

vector<char> Compiler::Optimize()
{
    return code;
    /*
    vector<char> out_code;
    for (int i = 0; i < code.size(); i++)
    {
        char c = code[i];
        out_code.push_back(c);
    }
    return out_code;*/
}

vector<char> Compiler::GetDump()
{
    vector<char> out_code;
    Function *func = table.FindFunction("Main");
    if (func == NULL)
    {
        cout << "Error: No 'Main' function found" << endl;
        return out_code;
    }

    PushData(func->Location(), out_code);
    DumpSysCalls(out_code);
    DumpTypes(out_code);
    
    vector<char> optimized = Optimize();
    out_code.insert(out_code.end(), 
        optimized.begin(), optimized.end());
    return out_code;
}

// Compiles a new file
// := include "<file_path>"
void Compiler::CompileInclude(Tokenizer *tk)
{
    tk->Match("include", TkType::Include);
    Token file_path = tk->Match("String", TkType::String);

    // If the file has already been compiled, then ignore this statement
    if (find(compiled_files.begin(), compiled_files.end(), 
        file_path.data) == compiled_files.end())
    {
        Compile(file_path.data);
        compiled_files.push_back(file_path.data);
    }
}

// Compiles a global function
// := func <Name>(<Args>, ...) <Block>
void Compiler::CompileFunc(Tokenizer *tk)
{
    tk->Match("func", TkType::Function);
    Token name = tk->Match("Name", TkType::Name);
    
    Function *func = new Function(name.data, 
        code.size(), table, tk);
    func->Compile();
    
    // Append function code to main code, then add function to symbol table
    vector<char> func_code = func->OutputCode();
    code.insert(code.end(), func_code.begin(), func_code.end());
    table.AddFunction(func);
}

// Marks a function name as a system call
// := syscall <Name>
void Compiler::CompileSysCall(Tokenizer *tk)
{
    tk->Match("syscall", TkType::SysCall);
    Token name = tk->Match("Name", TkType::Name);

    table.AddFunction(new Function(name.data, syscalls.size()));
    syscalls.push_back(name.data);
}

// Compiles a new class data struct
// := class <Name> { <Data> ... }
void Compiler::CompileClass(Tokenizer *tk)
{
	tk->Match("class", TkType::Class);
	Token name = tk->Match("Name", TkType::Name);
	
	Class *c = new Class(name.data, tk, &code, table);
	c->Compile();
	table.AddClass(c);
}

// Marks a class name as a system class
// := sysclass <Name> { <Syscall> ... }
void Compiler::CompileSysClass(Tokenizer *tk)
{
    tk->Match("sysclass", TkType::SysClass);
    Token name = tk->Match("Name", TkType::Name);

    Class *c = new Class(name.data, tk, &code, table);
    tk->Match("{", TkType::OpenBlock);
    while (tk->LookType() != TkType::CloseBlock)
    {
        // Match a syscall statement (syscall <Name>) and add it as a method
        tk->Match("syscall", TkType::SysCall);
        Token call = tk->Match("Name", TkType::Name);
        c->AddMethod(new Function(call.data, syscalls.size()));
        syscalls.push_back(name.data + ":" + call.data);
    }
    tk->Match("}", TkType::CloseBlock);
    table.AddClass(c);
}

Compiler::~Compiler()
{
    // Free symbol data
    table.CleanUp();
}
