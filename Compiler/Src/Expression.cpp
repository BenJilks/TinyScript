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
vector<char> Expression::CompileFunction(string name)
{
    Function *func = table->FindFunction(name);
    if (func == NULL)
    {
        tk->Error("No function called '" + name + "' found");
        return vector<char>();
    }
    return CompileCallFunc(func);
}

// Compiles a function call of know origin
vector<char> Expression::CompileCallFunc(Function *func)
{
    // Push args
    vector<char> code;
    int arg_length = 0;
    tk->Match("(", TkType::OpenArg);
    while (tk->LookType() != TkType::CloseArg)
    {
        arg_length++;
        AppendTo(code, Compile());
        if (tk->LookType() != TkType::Next)
            break;
        tk->Match(",", TkType::Next);
    }
    tk->Match(")", TkType::CloseArg);

    // Call function
    if (func->IsSysCall())
        code.push_back((char)ByteCode::CALL_SYS);
    else
        code.push_back((char)ByteCode::CALL);
    PushData(func->Location(), code);

    // Clean up arguments from stack after function call
    code.push_back((char)ByteCode::POP_ARGS);
    code.push_back(arg_length);
    return code;
}

// TODO: Explain what this does
void Expression::ParsePathNode(ExpressionPath& node, Class *pre_type)
{
    tk->Match(".", TkType::Path);
    Token class_name = tk->Match("Name", TkType::Name);
    
    if (tk->LookType() == TkType::Of)
    {
        tk->Match(":", TkType::Of);
        Token var_name = tk->Match("Name", TkType::Name);
        node.c = table->FindClass(class_name.data);
        node.var = var_name.data;
        return;
    }

    if (pre_type == NULL)
        tk->Error("Must state class type for '" + class_name.data + "' in non static var");
    node.c = pre_type;
    node.var = class_name.data;
}

// TODO: Explain what this does
void Expression::ParseFunctionNode(ExpressionPath& node)
{
    Function *func = node.c->FindMethod(node.var);
    if (func == NULL)
    {
        tk->Error("No method called '" + node.var + "' in '" + 
            node.c->Name() + "' found");
        return;
    }
    node.code = CompileCallFunc(func);
    node.is_func = true;
}

Class *Expression::GetNodeType(ExpressionPath node)
{
    if (node.is_func)
    {
        Function *func = node.c->FindMethod(node.var);
        if (func != NULL)
            return func->StaticType();
        return NULL;
    }
    
    Symbol *symb = node.c->FindAttribute(node.var);
    if (symb != NULL)
        return symb->type;
    return NULL;
}

ExpressionPath Expression::ParseFirstPath(string var)
{
    ExpressionPath first = (ExpressionPath){NULL, var};
    if (tk->LookType() == TkType::OpenArg)
    {
        Function *func = table->FindFunction(var);
        first.code = CompileCallFunc(func);
        first.is_func = true;
    }
    return first;
}

// TODO: Explain what this does
vector<ExpressionPath> Expression::CompilePath(string var)
{
    vector<ExpressionPath> path;
    path.push_back(ParseFirstPath(var));

    Symbol *symb = table->FindSymbol(var);
    Class *pre_type = symb == NULL ? NULL : symb->type;
    while (tk->LookType() == TkType::Path)
    {
        ExpressionPath node;
        ParsePathNode(node, pre_type);

        if (node.c != NULL)
        {
            node.is_func = false;
            if (tk->LookType() == TkType::OpenArg)
                ParseFunctionNode(node);
            path.push_back(node);
            pre_type = GetNodeType(node);
        }
    }

    return path;
}

vector<char> Expression::GenFirstPath(ExpressionPath first)
{
    vector<char> code;
    if (first.is_func)
    {
        AppendTo(code, first.code);
    }
    else
    {
        int location = table->FindLocation(first.var);
        code.push_back((char)ByteCode::PUSH_LOC);
        code.push_back((char)location);
    }

    return code;
}

// Push the value of a path to the stack
vector<char> Expression::GenPushPath(vector<ExpressionPath> path)
{
    vector<char> code = GenFirstPath(path[0]);
    for (int i = 1; i < path.size(); i++)
    {
        ExpressionPath node = path[i];
        if (node.is_func)
        {
            // Call method, and pop extra 'self' arg
            AppendTo(code, node.code);
            code.push_back((char)ByteCode::POP_ARGS);
            code.push_back((char)1);
        }
        else
        {
            // Push the attrubute of last object on stack
            code.push_back((char)ByteCode::PUSH_ATTR);

            int index = node.c->IndexOf(node.var);
            if (index == numeric_limits<int>::max())
                tk->Error("No attribute '" + node.var + "' in class '" + 
                    node.c->Name() + "' found");
            code.push_back((char)index);
        }
    }

    return code;
}

// Assing value on stack to the end of the path
vector<char> Expression::GenAssignPath(vector<ExpressionPath> path)
{
    vector<char> code;
    ExpressionPath last = path[path.size() - 1];

    // If the path only has one element, then assign to the local var
    if (path.size() == 1)
    {
        // If the local var does not exist, then create one
        if (table->FindSymbol(last.var) == NULL)
            table->AddLocal(last.var);
        
        int index = table->FindLocation(last.var);
        code.push_back((char)ByteCode::ASSIGN);
        code.push_back((char)index);
        return code;
    }

    // Remove last element and push to stack
    path.pop_back();
    code = GenPushPath(path);

    // Assign to attribute in last element on stack
    code.push_back((char)ByteCode::ASSIGN_ATTR);
    code.push_back((char)last.c->IndexOf(last.var));
    return code;
}

// Does the path end in a function call, cannot be assigned if it does
bool Expression::IsPathFunc(vector<ExpressionPath> path)
{
    ExpressionPath last = path[path.size() - 1];
    return last.is_func;
}

// Allocate a new object and push it onto stack
void Expression::CompileNewObject(Class *c, Node *node)
{
    int class_id = table->FindClassLocation(c);
    node->code.push_back((char)ByteCode::MALLOC);
    node->code.push_back((char)class_id);

    // Find a method named the same as the class
    Function *init = c->FindMethod(c->Name());

    // If found, then use as constructor
    if (init != NULL)
    {
        AppendTo(node->code, CompileCallFunc(init));
        node->code.push_back((char)ByteCode::POP);
        node->code.push_back((char)1);
    }
}

// Push a non-const value to the stack
void Expression::CompileName(Node *node)
{
    node->is_literal = false;
    Token name = tk->Match("Name", TkType::Name);

    // If it's a name of a class, then create a new instance
    Class *c = table->FindClass(name.data);
    if (c != NULL)
    {
        CompileNewObject(c, node);
        return;
    }

    // If the next token is an open argument, then call a function
    if (tk->LookType() == TkType::OpenArg)
    {
        AppendTo(node->code, CompileFunction(name.data));
        return;
    }

    // Otherwise it's a local
    vector<ExpressionPath> path = CompilePath(name.data);
    AppendTo(node->code, GenPushPath(path));
}

void Expression::CompileConst(Node *node)
{
    node->literal = tk->NextLook();
    node->is_literal = true;
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
        case TkType::Bool: CompileConst(node); break;
        case TkType::String: CompileConst(node); break;
        default: tk->Error("Token '" + tk->LookData() + "' is not a value"); break;
    }
    return node;
}

// Compile the lowest level operations (*, /)
Node *Expression::CompileFactor()
{
    Node *left = CompileTerm();
    while (tk->LookType() == TkType::Mul || tk->LookType() == TkType::Div)
    {
        Node *node = new Node();
        node->left = left;
        node->op = tk->NextLook();
        node->right = CompileTerm();
        left = node;
    }
    return left;
}

// Compile heigher level operations (+, -, ==, >, <)
Node *Expression::CompileExpression()
{
    Node *left = CompileFactor();
    while (tk->LookType() == TkType::Add || tk->LookType() == TkType::Sub || 
        tk->LookType() == TkType::Equals || tk->LookType() == TkType::GreaterThan || 
        tk->LookType() == TkType::LessThan)
    {
        Node *node = new Node();
        node->left = left;
        node->op = tk->NextLook();
        node->right = CompileFactor();
        left = node;
    }

    return left;
}

// Gen the code for pushing a value to the stack
vector<char> Expression::GenLiteral(Token literal)
{
    vector<char> code;
    switch (literal.type)
    {
        case TkType::Int:
            code.push_back((char)ByteCode::PUSH_INT);
            PushData(stoi(literal.data), code);
            break;

        case TkType::Float:
            code.push_back((char)ByteCode::PUSH_FLOAT);
            PushData(stof(literal.data), code);
            break;
        
        case TkType::Bool:
            code.push_back((char)ByteCode::PUSH_BOOL);
            code.push_back((char)(literal.data == "true" ? 1 : 0));
            break;
        
        case TkType::String:
            code.push_back((char)ByteCode::PUSH_STRING);
            code.push_back((char)literal.data.size());
            code.insert(code.end(), literal.data.begin(), literal.data.end());
            break;
        
        default: break;
    }
    return code;
}

// Gen the code for an operation
vector<char> Expression::GenCode(Node *node)
{
    if (node->left == NULL && node->right == NULL)
    {
        if (node->is_literal)
            return GenLiteral(node->literal);
        return node->code;
    }

    vector<char> code;
    AppendTo(code, GenCode(node->left));
    AppendTo(code, GenCode(node->right));

    switch(node->op.type)
    {
        case TkType::Add: code.push_back((char)ByteCode::ADD); break;
        case TkType::Sub: code.push_back((char)ByteCode::SUB); break;
        case TkType::Mul: code.push_back((char)ByteCode::MUL); break;
        case TkType::Div: code.push_back((char)ByteCode::DIV); break;
        case TkType::Equals: code.push_back((char)ByteCode::EQUALS); break;
        case TkType::GreaterThan: code.push_back((char)ByteCode::GREATERTHAN); break;
        case TkType::LessThan: code.push_back((char)ByteCode::LESSTHAN); break;
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

vector<char> Expression::Compile()
{
    Node *node = CompileExpression();
    vector<char> code = GenCode(node);

    CleanNodes(node);
    return code;
}
