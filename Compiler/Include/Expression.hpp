#pragma once
#include "Symbol.hpp"
#include "Tokenizer.hpp"

struct Node
{
    // Operation data
    Node *left;
    Node *right;
    Token op;

    // Value data
    vector<char> code;
    Token literal;
    bool is_literal;
};

struct ExpressionPath
{
    Class *c;
    string var;
    vector<char> code;
    bool is_func;
};

class Expression
{
public:
    Expression() {}

    Expression(SymbolTable *table, Tokenizer *tk) :
        table(table), tk(tk) {}
    vector<char> Compile();
    vector<char> CompileFunction(string name);

    vector<ExpressionPath> CompilePath(string var);
    vector<char> GenPushPath(vector<ExpressionPath> path);
    vector<char> GenAssignPath(vector<ExpressionPath> path);
    bool IsPathFunc(vector<ExpressionPath> path);

private:
    ExpressionPath ParseFirstPath(string var);
    void ParsePathNode(ExpressionPath& node, Class *pre_type);
    void ParseFunctionNode(ExpressionPath& node);
    Class *GetNodeType(ExpressionPath node);
    vector<char> GenFirstPath(ExpressionPath first);

    Node *CompileParentheses();
    Node *CompileTerm();
    Node *CompileFactor();
    Node *CompileExpression();
    void CleanNodes(Node *node);
    
    void CompileNewObject(Class *c, Node *node);
    void CompileName(Node *node);
    void CompileConst(Node *node);
    
    vector<char> CompileCallFunc(Function *func);
    vector<char> GenCode(Node *node);
    vector<char> GenLiteral(Token literal);

    SymbolTable *table;
    Tokenizer *tk;
};
