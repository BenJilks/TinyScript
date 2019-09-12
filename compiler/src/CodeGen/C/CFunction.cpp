#include "flags.h"
#if ARC_C

#include "CodeGen/CCode.hpp"
using namespace TinyScript::C;

void Code::compile_return(NodeReturn *node)
{
    ExpressionData data = compile_expression(node->get_value());
    write_line("return " + data.value + ";");
}

void Code::compile_assign(NodeAssign *node)
{
    NodeExpression *lvalue = node->get_left();
    NodeExpression *rvalue = node->get_right();
    if (rvalue) rvalue->symbolize();
    lvalue->symbolize();

    ExpressionData ldata = compile_expression(lvalue);
    string assign = ldata.value;
    if (rvalue)
    {
        ExpressionData rdata = compile_expression(rvalue);
        assign += " = " + rdata.value;
    }
    write_line(assign + ";");
}

void Code::compile_let(NodeLet *node)
{
    NodeExpression *value = node->get_value();
    if (value != nullptr)
        value->symbolize();
    node->symbolize();

    Symbol symb = node->get_symb();
    string let = compile_local_type(symb.type, symb.name);

    if (value != nullptr)
    {
        ExpressionData data = compile_expression(value);
        let += " = " + data.value;
    }
    write_line(let + ";");
}

void Code::compile_block(NodeBlock *node)
{
    for (int i = 0; i < node->get_child_size(); i++)
    {
        Node *child = (*node)[i];
        switch (child->get_type())
        {
            case NodeType::Let: compile_let((NodeLet*)child); break;
            case NodeType::Assign: compile_assign((NodeAssign*)child); break;
            case NodeType::Return: compile_return((NodeReturn*)child); break;
        }
    }
}

string Code::compile_function_symbol(Symbol symb)
{
    NodeFunction *node = nullptr;
    DataType return_type = symb.type;
    string name = symb.name;

    if (symb.parent != nullptr)
        if (symb.parent->get_type() == NodeType::Function)
            node = (NodeFunction*)symb.parent;

    string func = compile_param_type(return_type) + " " + name + "(";
    for (int i = 0; i < symb.params.size(); i++)
    {
        DataType type = symb.params[i];
        string name = node ? node->get_params()[i].name.data : "";

        func += compile_local_type(type, name);
        if (i < symb.params.size() - 1)
            func += ", ";
    }
    return func + ")";
}

void Code::compile_function(NodeFunction *node)
{
    if (node->is_template())
        return;

    string header = compile_function_symbol(node->get_symb());
    write_line("// func " + Symbol::printout(node->get_symb()));
    write_line(header);
    write_line("{");
    start_scope();
    compile_block((NodeBlock*)node);
    end_scope();
    write_line("}\n");
}

#endif
