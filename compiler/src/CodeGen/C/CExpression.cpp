#include "flags.h"
#if ARC_C

#include "CodeGen/CCode.hpp"
using namespace TinyScript::C;

ExpressionData Code::compile_expression(NodeExpression *node)
{
    ExpressionData data;
    data.temp_count = 0;

    node->symbolize();
    data.value = compile_expresion_node(data, node->get_data());
    return data;
}

string Code::compile_operation(ExpressionData &data, ExpDataNode *node)
{
    string left = compile_expresion_node(data, node->left);
    string right = compile_expresion_node(data, node->right);

    string op = "error";
    switch (node->token.type)
    {
        case TokenType::Add: op = "+"; break;
        case TokenType::Subtract: op = "-"; break;
        case TokenType::Multiply: op = "*"; break;
        case TokenType::Divide: op = "/"; break;
    }

    return "(" + left + " " + op + " " + right + ")";
}

string Code::compile_name(ExpressionData &data, ExpDataNode *node)
{
    string out = node->token.data;
    if (node->flags & NODE_ARGS_LIST)
    {
        out += "(";
        for (int i = 0; i < node->args.size(); i++)
        {
            out += compile_expresion_node(data, node->args[i]);
            if (i < node->args.size() - 1)
                out += ", ";
        }
        out += ")";
    }

    return out;
}

string Code::compile_array(ExpressionData &data, ExpDataNode *node)
{
    string out = "{";
    for (int i = 0; i < node->args.size(); i++)
    {
        ExpDataNode *value = node->args[i];
        out += compile_expresion_node(data, value);
        if (i < node->args.size() - 1)
            out += ", ";
    }

    return out + "}";
}

string Code::compile_rterm(ExpressionData &data, ExpDataNode *node)
{
    string value = node->token.data;
    switch (node->token.type)
    {
        case TokenType::Int: return value;
        case TokenType::Float: return value + "f";
        case TokenType::Char: return "'" + value + "'";
        case TokenType::Bool: return value == "true" ? "1" : "0";
        case TokenType::String: return "\"" + value + "\"";
        case TokenType::Name: return compile_name(data, node);
        case TokenType::OpenIndex: return compile_array(data, node);
        case TokenType::Ref: return "&" + compile_expresion_node(data, node->left);
        case TokenType::Copy: return "*" + compile_expresion_node(data, node->left);
    }

    return "error";
}

string Code::compile_expresion_node(ExpressionData &data, ExpDataNode *node)
{
    if (node->flags & NODE_OPERATION)
        return compile_operation(data, node);
    
    if (node->flags & NODE_INDEX)
    {
        return compile_expresion_node(data, node->left) + 
            "[" + compile_expresion_node(data, node->right) + "]";
    }

    if (node->flags & NODE_IN)
    {
        return compile_expresion_node(data, node->left) + 
            "." + compile_expresion_node(data, node->right);
    }

    return compile_rterm(data, node);
}

#endif
