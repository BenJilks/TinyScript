#include "Compiler.hpp"
#include "Class.hpp"
#include "Function.hpp"
#include "Bytecode.hpp"
#include "Debug.hpp"
#include <iostream>
#include <algorithm>
#include <tuple>

bool Compiler::Compile(string file_path)
{
    LOG("Compiling file '%s'\n", file_path.c_str());
    START_SCOPE();
    Tokenizer tk(file_path);
    globals = global_scope.Globals();

    while (tk.LookType() != TkType::EndOfFile && !tk.HasError())
    {
        switch(tk.LookType())
        {
            case TkType::Include: CompileInclude(&tk); break;
            case TkType::External: CompileExternal(&tk); break;
            case TkType::Function: CompileFunc(&tk); break;
            case TkType::Class: CompileClass(&tk); break;
            default: tk.Error("Unexpect token '" + tk.LookData() + "' in global scope"); break;
        }
    }
    
    END_SCOPE();
    LOG("Finished compiling '%s'\n", file_path.c_str());
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

void Compiler::DumpExternals(CodeGen &out_code)
{
    out_code.Argument((char)externals.size());
    for (External extenral : externals)
    {
        vector<string> calls = get<1>(extenral);
        out_code.String(get<0>(extenral));
        out_code.Argument((char)calls.size());
        for (string call : calls)
            out_code.String(call);
    }
}

void Compiler::DumpSysCalls(CodeGen &out_code)
{
    out_code.Argument((char)syscalls.size());
    for (string call : syscalls)
        out_code.String(call);
}

static void DumpOperator(Class *c, string name, CodeGen &out_code)
{
    Symbol *func = c->Attrs()->FindSymbol(name);
    if (func == NULL)
        out_code.Argument(-1);
    else
        out_code.Argument(func->Location());
}

void Compiler::DumpTypes(CodeGen &out_code)
{
    char count = classes.size();
    out_code.Argument((char)count);

    // Push class data:
    // string: name, int: size
    for (Class *c : classes)
    {
        out_code.String(c->Name());
        out_code.Argument(c->Size());
        out_code.Argument(c->IsSysClass());
        DumpOperator(c, "operator_add", out_code);
        DumpOperator(c, "operator_subtract", out_code);
        DumpOperator(c, "operator_multiply", out_code);
        DumpOperator(c, "operator_divide", out_code);
        DumpOperator(c, "operator_to_string", out_code);
        DumpOperator(c, "operator_get_index", out_code);
        DumpOperator(c, "operator_set_index", out_code);
        DumpOperator(c, "operator_it", out_code);
    }
}

CodeGen Compiler::Optimize()
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
    CodeGen out_code;
    Symbol *func = globals->FindSymbol("main");
    if (func == NULL)
    {
        cout << "Error: No 'main' function found" << endl;
        return vector<char>();
    }

    out_code.Argument(func->Location());
    DumpExternals(out_code);
    DumpSysCalls(out_code);
    DumpTypes(out_code);
    
    CodeGen optimized = Optimize();
    out_code.Append(optimized);
    return out_code.GetCode();
}

// Compiles a new file
// := include "<file_path>"
void Compiler::CompileInclude(Tokenizer *tk)
{
    tk->Match("include", TkType::Include);
    Token file_path = tk->Match("String", TkType::String);
    LOG("Include for '%s'\n", file_path.data.c_str());

    // If the file has already been compiled, then ignore this statement
    if (find(compiled_files.begin(), compiled_files.end(), 
        file_path.data) == compiled_files.end())
    {
        Compile(file_path.data);
        compiled_files.push_back(file_path.data);
    }
}

void Compiler::CompileExternal(Tokenizer *tk)
{
    tk->Match("external", TkType::External);
    Token file_path = tk->Match("String", TkType::String);
    LOG("External for lib '%s'\n", file_path.data.c_str());

    vector<string> external;
    tk->Match("{", TkType::OpenBlock);
    while (tk->LookType() != TkType::CloseBlock)
    {
        switch(tk->LookType())
        {
            case TkType::SysCall: CompileExternalCall(tk, external); break;
            case TkType::SysClass: CompileExternalClass(tk, external); break;
        }
    }
    tk->Match("}", TkType::CloseBlock);
    externals.push_back(make_tuple(file_path.data, external));
}

void Compiler::CompileExternalCall(Tokenizer *tk, vector<string> &calls)
{
    tk->Match("syscall", TkType::SysCall);
    Token name = tk->Match("Name", TkType::Name);
    calls.push_back(name.data);
    globals->MakeFunc(name.data, pointer++, true, NULL);
}

void Compiler::CompileExternalClass(Tokenizer *tk, vector<string> &calls)
{
    tk->Match("sysclass", TkType::SysClass);
    Token name = tk->Match("Name", TkType::Name);

    Class *c = new Class(name.data, tk, NULL, NULL);
    c->CompileSys(calls, &pointer);
    global_scope.MakeType(name.data, c->Attrs());
    classes.push_back(c);
}

// Compiles a global function
// := func <Name>(<Args>, ...) <Block>
void Compiler::CompileFunc(Tokenizer *tk)
{
    tk->Match("func", TkType::Function);
    Token name = tk->Match("Name", TkType::Name);
    LOG("Function '%s'\n", name.data.c_str());
    
    Function func(name.data, code.CurrPC(), &global_scope, tk, NULL);
    func.Compile();
    
    // Append function code to main code, then add function to symbol table
    CodeGen func_code = func.OutputCode();
    code.Append(func_code);
    Symbol *symb = globals->MakeFunc(name.data, func.Location(), false, NULL);
    symb->AssignType(func.ReturnType());
}

// Compiles a new class data struct
// := class <Name> { <Data> ... }
void Compiler::CompileClass(Tokenizer *tk)
{
	tk->Match("class", TkType::Class);
	Token name = tk->Match("Name", TkType::Name);
    LOG("Class '%s'\n", name.data.c_str());
	
	Class *c = new Class(name.data, tk, &code, &global_scope);
    global_scope.MakeType(name.data, c->Attrs());
	c->Compile();
    classes.push_back(c);
}

Compiler::~Compiler()
{
    for (Class *c : classes)
        delete c;
}
