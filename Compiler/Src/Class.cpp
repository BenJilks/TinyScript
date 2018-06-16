#include "Class.hpp"
#include "Function.hpp"
#include "Bytecode.hpp"
#include <algorithm>

Class::Class(string name, Tokenizer *tk, vector<char>* code, SymbolTable table) :
    name(name), tk(tk), code(code), table(table)
{
    this->table.AddClass(this);
}

Symbol *Class::FindAttribute(string name)
{
    for (Symbol *symb : attrs)
        if (symb->name == name)
            return symb;
    return NULL;
}

int Class::IndexOf(string var)
{
    Symbol *symb = FindAttribute(var);
    int attr_index = find(attrs.begin(), attrs.end(), symb) - attrs.begin();
    if (attr_index != attrs.size())
        return attr_index;
    
    return numeric_limits<int>::max();
}

Function *Class::FindMethod(string name)
{
    for (Function *func : methods)
        if (func->Name() == name)
            return func;
    return NULL;
}

void Class::Compile()
{
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
}

void Class::CompileAttr()
{
    Token var = tk->Match("Name", TkType::Name);
    Symbol *symb = new Symbol();
    symb->name = var.data;
    symb->is_const = false;
    symb->type = NULL;

    // Compile const class type
    if (tk->LookType() == TkType::Of)
    {
        tk->Match(":", TkType::Of);
        Token type = tk->Match("Name", TkType::Name);

        symb->is_const = true;
        symb->is_prim = true;
        symb->prim_type = SymbolTable::GetPrimType(type.data);
        if (symb->prim_type == (int)Primitive::OBJECT)
        {
            symb->type = table.FindClass(type.data);
            symb->is_prim = false;
        }
    }
    attrs.push_back(symb);
}

void Class::CompileMethod()
{
    tk->Match("func", TkType::Function);
    Token name = tk->Match("Name", TkType::Name);

    Function *func = new Function(name.data, 
        code->size(), table, tk);
    func->Compile();
    
    vector<char> func_code = func->OutputCode();
    code->insert(code->end(), func_code.begin(), func_code.end());
    methods.push_back(func);
}
