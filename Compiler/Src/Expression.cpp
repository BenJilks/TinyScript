#include "Expression.hpp"
#include "Bytecode.hpp"
#include "Function.hpp"
#include "Class.hpp"
#include <limits>
#include <string.h>

template<typename T>
static void PushData(T data, vector<char>& code)
{
    char *bytes = (char*)&data;
    code.insert(code.end(), bytes, 
        bytes + sizeof(T));
}

static inline void AppendTo(vector<char>& dest, vector<char> src)
{
    dest.insert(dest.end(), src.begin(), src.end());
}

// Compiles a global function call
CodeGen Expression::CompileFunction(string name)
{
    Symbol *func = scope->FindSymbol(name);
    if (func == NULL)
    {
        tk->Error("No function called '" + name + "' found");
        return CodeGen();
    }
    return CompileCallFunc(func);
}

int Expression::CompileFuncArgs(CodeGen &code)
{
    int arg_length = 0;
    tk->Match("(", TkType::OpenArg);
    while (tk->LookType() != TkType::CloseArg)
    {
        arg_length++;
        code.Append(Compile());
        if (tk->LookType() != TkType::Next)
            break;
        tk->Match(",", TkType::Next);
    }
    tk->Match(")", TkType::CloseArg);
    return arg_length;
}

// Compiles a function call of know origin
CodeGen Expression::CompileCallFunc(Symbol *func)
{
    // Push args
    CodeGen code;
    int arg_length = CompileFuncArgs(code);

    // Call function
    if (func->IsSystem())
        code.Instruction(ByteCode::CALL_SYS);
    else
        code.Instruction(ByteCode::CALL);
    code.Argument(func->Location());

    // Clean up arguments from stack after function call
    code.Instruction(ByteCode::POP_ARGS);
    code.Argument((char)arg_length);
    return code;
}

// TODO: Explain what this does
void Expression::ParsePathNode(ExpressionPath& node, SymbolType *pre_type)
{
    tk->Match(".", TkType::Path);
    Token class_name = tk->Match("Name", TkType::Name);
    
    if (tk->LookType() == TkType::Of)
    {
        tk->Match(":", TkType::Of);
        Token var_name = tk->Match("Name", TkType::Name);
        node.c = global->Type(class_name.data);
        node.var = var_name.data;
        return;
    }

    if (pre_type == NULL)
        tk->Error("Must state class type for '" + class_name.data + "' in non static var");
    node.c = pre_type;
    node.var = class_name.data;
    node.type = PathType::Attr;
}

void Expression::ParsePathIndex(ExpressionPath& node)
{
    tk->Match("[", TkType::OpenIndex);
    node.code = Compile();
    node.type = PathType::Index;
    tk->Match("]", TkType::CloseIndex);
}

// TODO: Explain what this does
void Expression::ParseFunctionNode(ExpressionPath& node)
{
    Symbol *func = node.c->Attr(node.var);
    if (func == NULL)
    {
        tk->Error("No method called '" + node.var + "' in '" + 
            node.c->Name() + "' found");
        return;
    }
    node.code = CompileCallFunc(func);
    node.type = PathType::Func;
}

SymbolType *Expression::GetNodeType(ExpressionPath node)
{
    if (node.c == NULL)
        return NULL;

    if (node.type == PathType::Func)
    {
        Symbol *func = node.c->Attr(node.var);
        if (func != NULL)
            return func->Type();
        return NULL;
    }
    
    Symbol *symb = node.c->Attr(node.var);
    if (symb != NULL)
        return symb->Type();
    return NULL;
}

ExpressionPath Expression::ParseFirstPath(string var)
{
    ExpressionPath first = (ExpressionPath){NULL, var};
    first.type = PathType::Attr;

    if (tk->LookType() == TkType::OpenArg)
    {
        Symbol *func = scope->FindSymbol(var);
        if (func == NULL)
        {
            tk->Error("Could not find function named '" + var + "'");
            return first;
        }
        first.code = CompileCallFunc(func);
        first.type = PathType::Func;
    }
    return first;
}

// TODO: Explain what this does
vector<ExpressionPath> Expression::CompilePath(string var)
{
    vector<ExpressionPath> path;
    path.push_back(ParseFirstPath(var));

    Symbol *symb = scope->FindSymbol(var);
    SymbolType *pre_type = symb == NULL ? NULL : symb->Type();
    while (tk->LookType() == TkType::Path || tk->LookType() == TkType::OpenIndex)
    {
        ExpressionPath node;
        if (tk->LookType() == TkType::OpenIndex)
        {
            node.c = NULL;
            ParsePathIndex(node);
        }
        else
        {
            ParsePathNode(node, pre_type);

            if (tk->LookType() == TkType::OpenArg)
                ParseFunctionNode(node);
        }

        path.push_back(node);
        pre_type = GetNodeType(node);
    }

    return path;
}

CodeGen Expression::GenSelfAttribute(ExpressionPath first)
{
    CodeGen code;
    Symbol *symb = attrs->FindSymbol(first.var);
    if (symb == NULL)
    {
        tk->Error("No variable named '" + first.var + 
            "' found in scope");
        return code;
    }

    Symbol *self = scope->FindSymbol("self");
    code.Instruction(ByteCode::PUSH_LOC);
    code.Argument((char)self->Location());

    code.Instruction(ByteCode::PUSH_ATTR);
    code.Argument((char)symb->Location());
    return code;
}

CodeGen Expression::GenFirstPath(ExpressionPath first)
{
    if (first.type == PathType::Func)
        return first.code;
    
    CodeGen code;
    Symbol *symb = scope->FindSymbol(first.var);
    if (symb == NULL)
    {
        if (attrs != NULL)
            return GenSelfAttribute(first);
        
        tk->Error("No variable named '" + first.var + 
            "' found in scope");
        return code;
    }

    int location = symb->Location();
    code.Instruction(ByteCode::PUSH_LOC);
    code.Argument((char)location);
    return code;
}

void Expression::GenPushAttr(CodeGen &code, ExpressionPath node)
{
    // Push the attrubute of last object on stack
    code.Instruction(ByteCode::PUSH_ATTR);

    Symbol *symb = node.c->Attr(node.var);
    if (symb == NULL)
    {
        tk->Error("The class '" + node.c->Name() + 
            "' does not have attribute '" + node.var + "'");
        return;
    }

    int index = symb->Location();
    if (index == numeric_limits<int>::max())
        tk->Error("No attribute '" + node.var + "' in class '" + 
            node.c->Name() + "' found");
    code.Argument((char)index);
}

void Expression::GenPushMethod(CodeGen &code, ExpressionPath node)
{
    // Call method, and pop extra 'self' arg
    code.Append(node.code);
    code.Instruction(ByteCode::POP_ARGS);
    code.Argument((char)1);
}

void Expression::GenPushIndex(CodeGen &code, ExpressionPath node)
{
    code.Append(node.code);
    code.Instruction(ByteCode::PUSH_INDEX);
}

// Push the value of a path to the stack
CodeGen Expression::GenPushPath(vector<ExpressionPath> path)
{
    CodeGen code = GenFirstPath(path[0]);
    for (int i = 1; i < path.size(); i++)
    {
        ExpressionPath node = path[i];
        switch(node.type)
        {
            case PathType::Attr: GenPushAttr(code, node); break;
            case PathType::Func: GenPushMethod(code, node); break;
            case PathType::Index: GenPushIndex(code, node); break;
        }
    }

    return code;
}

void Expression::AssignLast(CodeGen &code, ExpressionPath last)
{
    if (last.type == PathType::Index)
    {
        code.Append(last.code);
        code.Instruction(ByteCode::ASSIGN_INDEX);
    }
    else
    {
        Symbol *symb = last.c->Attr(last.var);
        // TODO: Error checking
        code.Instruction(ByteCode::ASSIGN_ATTR);
        code.Argument((char)symb->Location());
    }
}

CodeGen Expression::AssignSelfAttribute(Symbol *attr)
{
    CodeGen code;
    Symbol *self = scope->FindSymbol("self");
    code.Instruction(ByteCode::PUSH_LOC);
    code.Argument((char)self->Location());

    code.Instruction(ByteCode::ASSIGN_ATTR);
    code.Argument((char)attr->Location());
    return code;
}

// Assing value on stack to the end of the path
CodeGen Expression::GenAssignPath(vector<ExpressionPath> path)
{
    CodeGen code;
    ExpressionPath last = path[path.size() - 1];

    // If the path only has one element, then assign to the local var
    if (path.size() == 1)
    {
        // If the local var does not exist, then create one
        Symbol *symb = scope->FindSymbol(last.var);
        if (symb == NULL)
        {
            if (attrs != NULL)
            {
                Symbol *attr = attrs->FindSymbol(last.var);
                if (attr != NULL)
                    return AssignSelfAttribute(attr);
            }
            symb = scope->MakeLocal(last.var);
        }
        
        int index = symb->Location();
        code.Instruction(ByteCode::ASSIGN);
        code.Argument((char)index);
        return code;
    }

    // Remove last element and push to stack
    path.pop_back();
    code = GenPushPath(path);

    // Assign to attribute in last element on stack
    AssignLast(code, last);
    return code;
}

// Does the path end in a function call, cannot be assigned if it does
bool Expression::IsPathFunc(vector<ExpressionPath> path)
{
    ExpressionPath last = path[path.size() - 1];
    return last.type == PathType::Func;
}

// Allocate a new object and push it onto stack
void Expression::CompileNewObject(SymbolType *c, Node *node)
{
    int class_id = c->TypeID();
    node->code.Instruction(ByteCode::MALLOC);
    node->code.Argument((char)class_id);

    // Find a method named the same as the class
    Symbol *init = c->Attr(c->Name());

    // If found, then use as constructor
    if (init != NULL)
    {
        node->code.Append(CompileCallFunc(init));
        node->code.Instruction(ByteCode::POP);
        node->code.Argument((char)1);
    }
}

// Push a non-const value to the stack
void Expression::CompileName(Node *node)
{
    node->is_literal = false;
    Token name = tk->Match("Name", TkType::Name);

    // If it's a name of a class, then create a new instance
    SymbolType *c = global->Type(name.data);
    if (c != NULL)
    {
        CompileNewObject(c, node);
        return;
    }

    // If the next token is an open argument, then call a function
    if (tk->LookType() == TkType::OpenArg)
    {
        node->code.Append(CompileFunction(name.data));
        return;
    }

    // Otherwise it's a local
    vector<ExpressionPath> path = CompilePath(name.data);
    node->code.Append(GenPushPath(path));
}

void Expression::CompileConst(Node *node)
{
    node->literal = tk->NextLook();
    node->is_literal = true;
}

void Expression::CompileArray(Node *node)
{
    int size = 0;
    tk->Match("[", TkType::OpenIndex);
    while (tk->LookType() != TkType::CloseIndex)
    {
        size++;
        node->code.Append(Compile());
        if (tk->LookType() != TkType::CloseIndex)
            tk->Match(",", TkType::Next);
    }
    tk->Match("]", TkType::CloseIndex);

    node->is_literal = false;
    node->code.Instruction(ByteCode::MAKE_ARRAY);
    node->code.Argument((char)size);
}

// Compile an expression inside an expression, enclosed in parentheses
// ... + (a + b) + ... 
Node *Expression::CompileParentheses()
{
    tk->Match("(", TkType::OpenArg);
    Node *inside = CompileExpression();
    tk->Match(")", TkType::CloseArg);
    return inside;
}

// Compile a new value in an expression
Node *Expression::CompileTerm()
{
    if (tk->LookType() == TkType::OpenArg)
        return CompileParentheses();

    Node *node = new Node();
    node->left = NULL;
    node->right = NULL;

    switch (tk->LookType())
    {
        case TkType::Name: CompileName(node); break;
        case TkType::Int: CompileConst(node); break;
        case TkType::Float: CompileConst(node); break;
        case TkType::Char: CompileConst(node); break;
        case TkType::Bool: CompileConst(node); break;
        case TkType::String: CompileConst(node); break;
        case TkType::OpenIndex: CompileArray(node); break;
        default: tk->Error("Token '" + tk->LookData() + "' is not a value"); break;
    }
    return node;
}

// Compile the lowest level operations (*, /, ==, >, <)
Node *Expression::CompileFactor()
{
    Node *left = CompileTerm();
    while (tk->LookType() == TkType::Mul || tk->LookType() == TkType::Div || 
        tk->LookType() == TkType::Equals || tk->LookType() == TkType::GreaterThan || 
        tk->LookType() == TkType::GreaterThanEqual || tk->LookType() == TkType::LessThan ||
        tk->LookType() == TkType::LessThanEqual)
    {
        Node *node = new Node();
        node->left = left;
        node->op = tk->NextLook();
        node->right = CompileTerm();
        left = node;
    }
    return left;
}

// Compile heigher level operations (+, -)
Node *Expression::CompileExpression()
{
    Node *left = CompileFactor();
    while (tk->LookType() == TkType::Add || tk->LookType() == TkType::Sub)
    {
        Node *node = new Node();
        node->left = left;
        node->op = tk->NextLook();
        node->right = CompileFactor();
        left = node;
    }

    return left;
}

// Compile the logic operations (and, or)
Node *Expression::CompileLogic()
{
    Node *left = CompileExpression();
    while (tk->LookType() == TkType::And || tk->LookType() == TkType::Or)
    {
        Node *node = new Node();
        node->left = left;
        node->op = tk->NextLook();
        node->right = CompileExpression();
        left = node;
    }
    return left;
}

// Gen the code for pushing a value to the stack
CodeGen Expression::GenLiteral(Token literal)
{
    CodeGen code;
    switch (literal.type)
    {
        case TkType::Int:
            code.Instruction(ByteCode::PUSH_INT);
            code.Argument(stoi(literal.data));
            break;

        case TkType::Float:
            code.Instruction(ByteCode::PUSH_FLOAT);
            code.Argument(stof(literal.data));
            break;
        
        case TkType::Char:  
            code.Instruction(ByteCode::PUSH_CHAR);
            code.Argument(literal.data[0]);
            break;

        case TkType::Bool:
            code.Instruction(ByteCode::PUSH_BOOL);
            code.Argument((char)(literal.data == "true" ? 1 : 0));
            break;
        
        case TkType::String:
            code.Instruction(ByteCode::PUSH_STRING);
            code.String(literal.data);
            break;
        
        default: break;
    }
    return code;
}

// Gen the code for an operation
CodeGen Expression::GenCode(Node *node)
{
    if (node->left == NULL && node->right == NULL)
    {
        if (node->is_literal)
            return GenLiteral(node->literal);
        return node->code;
    }

    CodeGen code;
    code.Append(GenCode(node->left));
    code.Append(GenCode(node->right));

    switch(node->op.type)
    {
        case TkType::Add: code.Instruction(ByteCode::ADD); break;
        case TkType::Sub: code.Instruction(ByteCode::SUB); break;
        case TkType::Mul: code.Instruction(ByteCode::MUL); break;
        case TkType::Div: code.Instruction(ByteCode::DIV); break;
        case TkType::Equals: code.Instruction(ByteCode::EQUALS); break;
        case TkType::GreaterThan: code.Instruction(ByteCode::GREATERTHAN); break;
        case TkType::GreaterThanEqual: code.Instruction(ByteCode::GREATERTHANEQUAL); break;
        case TkType::LessThan: code.Instruction(ByteCode::LESSTHAN); break;
        case TkType::LessThanEqual: code.Instruction(ByteCode::LESSTHANEQUAL); break;
        case TkType::And: code.Instruction(ByteCode::AND); break;
        case TkType::Or: code.Instruction(ByteCode::OR); break;
        default: tk->Error("Token '" + node->op.data + "' is not an operation"); break;
    }
    return code;
}

void Expression::CleanNodes(Node *node)
{
    if (node->left != NULL)
        CleanNodes(node->left);
    if (node->right != NULL)
        CleanNodes(node->right);
    delete node;
}

CodeGen Expression::Compile()
{
    Node *node = CompileLogic();
    CodeGen code = GenCode(node);

    CleanNodes(node);
    return code;
}
