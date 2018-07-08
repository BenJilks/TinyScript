#include "Function.hpp"
#include "Class.hpp"
#include "Bytecode.hpp"
#include <algorithm>
#include <memory.h>

Function::Function(string name, int location, SymbolTable table, Tokenizer *tk) :
    name(name), location(location), table(table), tk(tk), is_syscall(false)
{
    expression = Expression(&this->table, tk);
    this->table.AddFunction(this);
}

void Function::Compile()
{
    CompileParams();
    CompileStaticReturn();
    CompileBlock();
}

void Function::CompileStaticReturn()
{
    // If the next token is ':' then read the type
    if (tk->LookType() == TkType::Of)
    {
        tk->Match(":", TkType::Of);
        Token type_name = tk->Match("Name", TkType::Name);

        int prim = SymbolTable::GetPrimType(type_name.data);
        if (prim == (int)Primitive::OBJECT)
            type = table.FindClass(type_name.data);
    }
}

// Read the function params and create arg symbols
void Function::CompileParams()
{
    tk->Match("(", TkType::OpenArg);
    while (tk->LookType() != TkType::CloseArg)
    {
        Token arg = tk->Match("Name", TkType::Name);
        table.AddArgument(arg.data);
        if (tk->LookType() == TkType::Of)
            AssignConstType(arg.data);
        
        if (tk->LookType() == TkType::CloseArg)
            break;
        tk->Match(",", TkType::Next);
    }
    tk->Match(")", TkType::CloseArg);
}

vector<char> Function::OutputCode()
{
    vector<char> out_code;

    // Allocate stack frame of function size
    out_code.push_back((char)ByteCode::ALLOC);
    out_code.push_back((char)table.Length());
    out_code.insert(out_code.end(), code.begin(), code.end());

    // Add default return statement for (int)0 at end of function
    out_code.push_back((char)ByteCode::PUSH_INT);
    out_code.push_back((char)0);
    out_code.push_back((char)0);
    out_code.push_back((char)0);
    out_code.push_back((char)0);
    out_code.push_back((char)ByteCode::RETURN);
    return out_code;
}

void Function::CompileBlock()
{
    // If the next token is '{' then read a multi line block
    if (tk->LookType() == TkType::OpenBlock)
    {
        tk->Match("{", TkType::OpenBlock);
        while (tk->LookType() != TkType::CloseBlock && !tk->HasError())
            CompileStatement();
        tk->Match("}", TkType::CloseBlock);
        return;
    }

    // Otherwise, read a single line block
    CompileStatement();
}

void Function::CompileStatement()
{
    switch(tk->LookType())
    {
        case TkType::Name: CompileAssign(); break;
        case TkType::Return: CompileReturn(); break;
        case TkType::If: CompileIf(); break;
        case TkType::While: CompileWhile(); break;
        case TkType::For: CompileFor(); break;
        default: tk->Error("Unexpected token '" + tk->LookData() + 
            "' in function '" + name + "'"); break;
    }
}

static inline void AppendTo(vector<char>& dest, vector<char> src)
{
    dest.insert(dest.end(), src.begin(), src.end());
}

template<typename T>
static void PushData(T data, vector<char>& code)
{
    char *bytes = (char*)&data;
    code.insert(code.end(), bytes, 
        bytes + sizeof(T));
}

// Read const type and assing to var
void Function::AssignConstType(string var)
{
    tk->Match(":", TkType::Of);
    Token type = tk->Match("Name", TkType::Name);
    
    Symbol *symb = table.FindSymbol(var);
    symb->is_const = true;
    symb->is_prim = true;
    symb->prim_type = SymbolTable::GetPrimType(type.data);
    if ((Primitive)symb->prim_type == Primitive::OBJECT)
    {
        symb->type = table.FindClass(type.data);
        symb->is_prim = false;
    }
}

// Compile a variable assign statment or function call
// := <var> (: <Type>) = <value>
// or
// := <func>(<args>, ...)
void Function::CompileAssign()
{
    Token left = tk->Match("Name", TkType::Name);
    if (tk->LookType() == TkType::Of)
    {
        // TODO: Error checking here... and everywhere
        table.AddLocal(left.data);
        AssignConstType(left.data);
    }

    // Parse path location and call function if path ends with one
    vector<ExpressionPath> path = expression.CompilePath(left.data);
    if (expression.IsPathFunc(path))
    {
        AppendTo(code, expression.GenPushPath(path));
        code.push_back((char)ByteCode::POP);
        code.push_back((char)1);
        return;
    }
    
    // Otherwise, assign value
    tk->Match("=", TkType::Assign);
    AppendTo(code, expression.Compile());
    AppendTo(code, expression.GenAssignPath(path));
}

// Returns from a function with a value
// := ret <Value>
void Function::CompileReturn()
{
    tk->Match("ret", TkType::Return);
    AppendTo(code, expression.Compile());
    code.push_back((char)ByteCode::RETURN);
}

// Only execute code if a condition is met
// := if <condition> <block>
void Function::CompileIf()
{
    tk->Match("if", TkType::If);
    AppendTo(code, expression.Compile());
    
    code.push_back((char)ByteCode::BRANCH_IF_NOT);
    code.push_back((char)0);
    int addr = code.size()-1;
    CompileBlock();
    code[addr] = code.size() - addr;
}

void Function::CompileIter(vector<ExpressionPath> path)
{
    string name = path[0].var;
    table.AddLocal(name);

    tk->Match("in", TkType::In);
    AppendTo(code, expression.Compile());
    code.push_back((char)ByteCode::MAKE_IT);
    int start = code.size();
    code.push_back((char)ByteCode::BRANCH_IF_IT);
    int addr = code.size();
    PushData(0, code);
    code.push_back((char)ByteCode::IT_NEXT);
    code.push_back((char)table.FindLocation(name));

    CompileBlock();

    code.push_back((char)ByteCode::BRANCH);
    PushData(start - code.size(), code);
    int diff = code.size() - addr;
    memcpy(&code[addr], &diff, sizeof(int));
    code.push_back((char)ByteCode::POP);
    code.push_back((char)1);
}

// Repeat a block of code an amount of times
// := for <var> = <start> to <end> <block>
void Function::CompileFor()
{
    tk->Match("for", TkType::For);
    Token var = tk->Match("Name", TkType::Name);

    // Parse and assing staring value
    vector<ExpressionPath> path = expression.CompilePath(var.data);
    if (tk->LookType() == TkType::In)
    {
        CompileIter(path);
        return;
    }
    tk->Match("=", TkType::Assign);
    AppendTo(code, expression.Compile());
    AppendTo(code, expression.GenAssignPath(path));
    tk->Match("to", TkType::To);

    // Test if value is less than end state, and jump to end if not
    int start = code.size();
    code.push_back((char)ByteCode::PUSH_LOC);
    code.push_back((char)table.FindLocation(var.data));
    AppendTo(code, expression.Compile());
    code.push_back((char)ByteCode::LESSTHAN);
    code.push_back((char)ByteCode::BRANCH_IF_NOT);
    int addr = code.size();
    PushData(0, code);

    CompileBlock();

    // Increment value
    code.push_back((char)ByteCode::INC_LOC);
    code.push_back((char)table.FindLocation(var.data));
    code.push_back((char)1);

    // Jump to start of loop
    code.push_back((char)ByteCode::BRANCH);
    PushData(start - code.size(), code);
    int diff = code.size() - addr;
    memcpy(&code[addr], &diff, sizeof(int));
}

void Function::CompileWhile()
{
    tk->Match("while", TkType::While);
    
    // Compile condition
    int start = code.size();
    AppendTo(code, expression.Compile());
    
    // Branch to end if condition is not met
    code.push_back((char)ByteCode::BRANCH_IF_NOT);
    int addr = code.size();
    PushData(0, code);

    CompileBlock();

    // Jump to start
    code.push_back((char)ByteCode::BRANCH);
    PushData(start - code.size(), code);
    int diff = code.size() - addr;
    memcpy(&code[addr], &diff, sizeof(int));
}

Function::~Function()
{
    table.PopScope();
}
