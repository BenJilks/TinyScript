#include "Function.hpp"
#include "Class.hpp"
#include "Bytecode.hpp"
#include "Debug.hpp"
#include <algorithm>
#include <memory.h>

Function::Function(string name, int location, GlobalScope *global, Tokenizer *tk, Scope *attrs) :
    name(name), location(location), scope(global), tk(tk), is_syscall(false), type(NULL)
{
    expression = Expression(&scope, tk, attrs);
    scope.MakeFunc(name, location, false, NULL);
}

void Function::AddSelf(SymbolType *type)
{
    scope.MakeParameter("self", type);
}

void Function::Compile()
{
    START_SCOPE();
    CompileParams();
    CompileStaticReturn();
    END_SCOPE();

    CompileBlock();
}

void Function::CompileStaticReturn()
{
    // If the next token is ':' then read the type
    if (tk->LookType() == TkType::Of)
    {
        tk->Match(":", TkType::Of);
        Token type_name = tk->Match("Name", TkType::Name);

        Symbol *func = scope.FindSymbol(name);
        GlobalScope *global_scope = scope.Global();
        type = global_scope->Type(type_name.data);
    }
}

// Read the function params and create arg symbols
void Function::CompileParams()
{
    tk->Match("(", TkType::OpenArg);
    while (tk->LookType() != TkType::CloseArg)
    {
        Token arg = tk->Match("Name", TkType::Name);
        SymbolType *type = ParseConstType();
        scope.MakeParameter(arg.data, type);
        LOG("Parameter '%s' of type '%s'\n", arg.data.c_str(), 
            type == NULL ? "none" : type->Name().c_str());
        
        if (tk->LookType() == TkType::CloseArg)
            break;
        tk->Match(",", TkType::Next);
    }
    tk->Match(")", TkType::CloseArg);
    scope.FinishParams();
}

CodeGen Function::OutputCode()
{
    CodeGen out_code;

    // Allocate stack frame of function size
    out_code.Instruction(ByteCode::ALLOC);
    out_code.Argument((char)scope.Length());
    out_code.Append(code);

    // Add default return statement for (int)0 at end of function
    out_code.Instruction(ByteCode::PUSH_INT);
    out_code.Argument((int)0);
    out_code.Instruction(ByteCode::RETURN);
    return out_code;
}

void Function::CompileBlock()
{
    START_SCOPE();

    // If the next token is '{' then read a multi line block
    if (tk->LookType() == TkType::OpenBlock)
    {
        tk->Match("{", TkType::OpenBlock);
        while (tk->LookType() != TkType::CloseBlock && !tk->HasError())
            CompileStatement();
        tk->Match("}", TkType::CloseBlock);
    }
    else
    {
        // Otherwise, read a single line block
        CompileStatement();
    }

    END_SCOPE();
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
SymbolType *Function::ParseConstType()
{
    if (tk->LookType() != TkType::Of)
        return NULL;

    tk->Match(":", TkType::Of);
    Token type_name = tk->Match("Name", TkType::Name);
    GlobalScope *global_scope = scope.Global();
    SymbolType *type = global_scope->Type(type_name.data);
    return type;
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
        Symbol *symb = scope.MakeLocal(left.data);
        symb->AssignType(ParseConstType());
    }

    // Parse path location and call function if path ends with one
    vector<ExpressionPath> path = expression.CompilePath(left.data);
    if (expression.IsPathFunc(path))
    {
        LOG("Call function '%s'\n", left.data.c_str());
        code.Append(expression.GenPushPath(path));
        code.Instruction(ByteCode::POP);
        code.Argument((char)1);
        return;
    }
    
    // Otherwise, assign value
    LOG("Assign '%s'\n", left.data.c_str());
    tk->Match("=", TkType::Assign);
    code.Append(expression.Compile());
    code.Append(expression.GenAssignPath(path));
}

// Returns from a function with a value
// := ret <Value>
void Function::CompileReturn()
{
    LOG("Returning\n");
    tk->Match("ret", TkType::Return);
    code.Append(expression.Compile());
    code.Instruction(ByteCode::RETURN);
}

// Only execute code if a condition is met
// := if <condition> <block>
void Function::CompileIf()
{
    LOG("If statement\n");
    tk->Match("if", TkType::If);
    code.Append(expression.Compile());
    
    code.Instruction(ByteCode::BRANCH_IF_NOT);
    int else_l = code.MakeLabel();
    CompileBlock();
    code.SetLabel(else_l);
}

void Function::CompileIter(vector<ExpressionPath> path)
{
    string name = path[0].var;
    Symbol *iter = scope.MakeLocal(name);

    tk->Match("in", TkType::In);
    code.Append(expression.Compile());
    code.Instruction(ByteCode::MAKE_IT);
    int start = code.CurrPC();
    code.Instruction(ByteCode::BRANCH_IF_IT);
    int end = code.MakeLabel();
    code.Instruction(ByteCode::IT_NEXT);
    code.Argument((char)iter->Location());

    CompileBlock();

    code.Instruction(ByteCode::BRANCH);
    code.Argument(start - code.CurrPC());
    code.SetLabel(end);
    code.Instruction(ByteCode::POP);
    code.Argument((char)1);
}

// Repeat a block of code an amount of times
// := for <var> = <start> to <end> <block>
void Function::CompileFor()
{
    LOG("For loop\n");
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
    code.Append(expression.Compile());
    code.Append(expression.GenAssignPath(path));
    tk->Match("to", TkType::To);
    Symbol *iter = scope.FindSymbol(var.data);

    // Test if value is less than end state, and jump to end if not
    int start = code.CurrPC();
    code.Instruction(ByteCode::PUSH_LOC);
    code.Argument((char)iter->Location());
    code.Append(expression.Compile());
    code.Instruction(ByteCode::LESSTHAN);
    code.Instruction(ByteCode::BRANCH_IF_NOT);
    int end = code.MakeLabel();

    CompileBlock();

    // Increment value
    code.Instruction(ByteCode::INC_LOC);
    code.Argument((char)iter->Location());
    code.Argument((char)1);

    // Jump to start of loop
    code.Instruction(ByteCode::BRANCH);
    code.Argument(start - code.CurrPC());
    code.SetLabel(end);
}

void Function::CompileWhile()
{
    LOG("While loop\n");
    tk->Match("while", TkType::While);
    
    // Compile condition
    int start = code.CurrPC();
    code.Append(expression.Compile());
    
    // Branch to end if condition is not met
    code.Instruction(ByteCode::BRANCH_IF_NOT);
    int end = code.MakeLabel();

    CompileBlock();

    // Jump to start
    code.Instruction(ByteCode::BRANCH);
    code.Argument(start - code.CurrPC());
    code.SetLabel(end);
}
