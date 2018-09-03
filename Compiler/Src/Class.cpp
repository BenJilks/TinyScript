#include "Class.hpp"
#include "Function.hpp"
#include "Bytecode.hpp"
#include "Debug.hpp"
#include <algorithm>

Class::Class(string name, vector<Function> *functions, Tokenizer *tk, GlobalScope *global) :
    name(name), functions(functions), tk(tk), global(global)
{
    attrs = new Scope(NULL);
    is_sys_class = true;
}

void Class::Compile()
{
    is_sys_class = false;
    
    START_SCOPE();
    tk->Match("{", TkType::OpenBlock);
	while (tk->LookType() != TkType::CloseBlock)
	{
        switch (tk->LookType())
        {
            case TkType::Name: CompileAttr(); break;
            case TkType::Function: CompileMethod(); break;
        }
	}
	tk->Match("}", TkType::CloseBlock);
    END_SCOPE();
}

void Class::CompileAttr()
{
    Token var = tk->Match("Name", TkType::Name);
    Symbol *symb = attrs->MakeLocal(var.data);
    LOG("Attribute '%s'\n", var.data.c_str());

    // Compile const class type
    if (tk->LookType() == TkType::Of)
    {
        tk->Match(":", TkType::Of);
        Token type_name = tk->Match("Name", TkType::Name);

        SymbolType *type = global->Type(type_name.data);
        // TODO: error checking
        symb->AssignType(type);
    }
}

void Class::CompileMethod()
{
    tk->Match("func", TkType::Function);
    Token name = tk->Match("Name", TkType::Name);
    LOG("Method '%s'\n", name.data.c_str());

    Function func(name.data, functions->size(), global, tk, attrs);
    func.AddSelf(global->Type(this->name));
    func.Compile();
    functions->push_back(func);
    
    Symbol *symb = attrs->MakeFunc(name.data, func.FuncID(), false, NULL);
    symb->AssignType(func.ReturnType());
}

void Class::CompileSys(vector<string> &syscalls, int *pointer)
{
    START_SCOPE();
    tk->Match("{", TkType::OpenBlock);
    while (tk->LookType() != TkType::CloseBlock)
    {
        // Match a syscall statement (syscall <Name>) and add it as a method
        tk->Match("syscall", TkType::SysCall);
        Token call = tk->Match("Name", TkType::Name);
        attrs->MakeFunc(call.data, (*pointer)++, true, NULL);
        syscalls.push_back(name + "_" + call.data);

        LOG("SysMethod '%s'\n", call.data.c_str());
    }
    tk->Match("}", TkType::CloseBlock);
    END_SCOPE();
}

Class::~Class()
{
}
