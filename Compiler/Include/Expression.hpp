#pragma once
#include "Symbol.hpp"
#include "Tokenizer.hpp"
#include "CodeGen.hpp"

struct Node
{
    // Operation data
    Node *left;
    Node *right;
    Token op;

    // Value data
    CodeGen code;
    Token literal;
    bool is_literal;
};

enum class PathType
{
    Attr,
    Func,
    Index
};

struct ExpressionPath
{
    SymbolType *c;
    string var;
    CodeGen code;
    PathType type;
};

class Expression
{
public:
    Expression() {}

    Expression(Scope *scope, Tokenizer *tk, Scope *attrs) :
        scope(scope), tk(tk), global(scope->Global()), attrs(attrs) {}
    CodeGen Compile();
    CodeGen CompileFunction(string name);

    vector<ExpressionPath> CompilePath(string var);
    CodeGen GenPushPath(vector<ExpressionPath> path);
    CodeGen GenAssignPath(vector<ExpressionPath> path);
    bool IsPathFunc(vector<ExpressionPath> path);

private:
    CodeGen PushLocal(Symbol *local);
    CodeGen PushLocalAttr(Symbol *local, Symbol *attr);
    ExpressionPath ParseFirstPath(string var);
    SymbolType *FindType(string var);
    void ParsePathNode(ExpressionPath& node, SymbolType *pre_type);
    void ParsePathIndex(ExpressionPath& node);
    void ParseFunctionNode(ExpressionPath& node);
    SymbolType *GetNodeType(ExpressionPath node);
    CodeGen GenSelfAttribute(ExpressionPath first);
    CodeGen AssignSelfAttribute(Symbol *attr);
    CodeGen GenFirstPath(ExpressionPath first);
    void GenPushAttr(CodeGen &code, ExpressionPath node);
    void GenPushMethod(CodeGen &code, ExpressionPath node);
    void GenPushIndex(CodeGen &code, ExpressionPath node);
    void AssignLast(CodeGen &code, ExpressionPath last);

    Node *CompileParentheses();
    Node *CompileLogic();
    Node *CompileTerm();
    Node *CompileFactor();
    Node *CompileExpression();
    void CleanNodes(Node *node);
    
    void CompileNewObject(SymbolType *c, Node *node);
    void CompileName(Node *node);
    void CompileConst(Node *node);
    void CompileArray(Node *node);
    
    int CompileFuncArgs(CodeGen &code);
    CodeGen CompileCallFunc(Symbol *func, bool is_method);
    CodeGen GenCode(Node *node);
    CodeGen GenLiteral(Token literal);

    Scope *scope;
    Scope *attrs;
    GlobalScope *global;
    Tokenizer *tk;
};
