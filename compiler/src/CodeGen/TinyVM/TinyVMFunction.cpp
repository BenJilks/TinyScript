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
    compile_block((NodeBlock*)node);
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
    int size = DataType::find_size(right->get_data_type());

    compile_rexpression(right);
    compile_lexpression(left);
    write_byte(BC_ASSIGN_REF_X);
    write_int(size);
}
