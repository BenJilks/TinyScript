#include "CodeGen/TinyVMCode.hpp"
extern "C"
{
#include "bytecode.h"
}
using namespace TinyScript::TinyVM;

void Code::compile_function(NodeFunction *node)
{
    Token name = node->get_name();
    Logger::log(name.debug_info, "Compiling function '" + name.data + "'");

    // Create stack frame
    assign_label(Symbol::printout(node->get_symb()));
    write_byte(BC_CREATE_FRAME);
    write_byte(node->get_scope_size());

    // Compile body
    compile_block((NodeBlock*)node);

    // Write default return statement
    write_byte(BC_RETURN);
    write_byte(0);
    write_byte(node->get_arg_size());
}

void Code::compile_return(NodeReturn *node)
{
    NodeExpression *value = node->get_value();
    NodeFunction *func = (NodeFunction*)node->get_parent(NodeType::Function);
    int size = DataType::find_size(value->get_data_type());

    compile_rexpression(value);
    write_byte(BC_RETURN);
    write_byte(size);
    write_byte(func->get_arg_size());
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
            case NodeType::If: compile_if((NodeIf*)child); break;
        }
    }
}

void Code::compile_let(NodeLet *node)
{
    Symbol symb = node->get_symb();
    NodeExpression *value = node->get_value();
    int size = DataType::find_size(value->get_data_type());

    compile_rexpression(value);
    write_byte(BC_STORE_LOCAL_X);
    write_int(size);
    write_byte(symb.location);
}

void Code::compile_assign(NodeAssign *node)
{
    NodeExpression *left = node->get_left();
    NodeExpression *right = node->get_right();

    if (right != nullptr)
    {
        int size = DataType::find_size(right->get_data_type());
        compile_rexpression(right);
        compile_lexpression(left);
        write_byte(BC_ASSIGN_REF_X);
        write_int(size);
    }
    else
    {
        int size = DataType::find_size(left->get_data_type());
        compile_rexpression(left);
        if (size > 0)
        {
            write_byte(BC_POP);
            write_byte(size);
        }
    }
}

void Code::compile_if(NodeIf *node)
{
    string end = gen_label();

    compile_rexpression(node->get_condition());
    write_byte(BC_JUMP_IF_NOT);
    write_label(end);
    compile_block((NodeBlock*)node);
    assign_label(end);
}
