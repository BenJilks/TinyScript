#include <memory.h>
#include "CodeGen/TinyVMCode.hpp"
extern "C"
{
#include "bytecode.h"
}
using namespace TinyScript::TinyVM;

void Code::compile_function(NodeFunction *node)
{
    // Don't compile tempate functions
    if (node->is_template() || node->is_compiled())
        return;

    Token name = node->get_name();
    Logger::log(name.debug_info, "Compiling function '" + 
        Symbol::printout(node->get_symb()) + "'");

    // Create stack frame
    assign_label(Symbol::printout(node->get_symb()));
    write_byte(BC_CREATE_FRAME);
    int scope_size_loc = code.size();
    write_int(0);

    // Compile body
    compile_block((NodeBlock*)node);

    // Write default return statement
    write_byte(BC_RETURN);
    write_byte(0);
    write_byte(node->get_arg_size());

    int scope_size = node->get_scope_size();
    memcpy(&code[scope_size_loc], &scope_size, sizeof(int));
    node->set_compiled();
}

void Code::compile_return(NodeReturn *node)
{
    Logger::log(node->get_value()->get_data()->token.debug_info, "Compile Return");

    NodeExpression *value = node->get_value();
    NodeFunction *func = (NodeFunction*)node->get_parent(NodeType::Function);
    int size;

    compile_rexpression(value);
    size = DataType::find_size(value->get_data_type());
    write_byte(BC_RETURN);
    write_byte(size);
    write_byte(func->get_arg_size());
}

void Code::compile_block(NodeBlock *node)
{
    Logger::start_scope();
    for (int i = 0; i < node->get_child_size(); i++)
    {
        Node *child = (*node)[i];
        switch (child->get_type())
        {
            case NodeType::Let: compile_let((NodeLet*)child); break;
            case NodeType::Assign: compile_assign((NodeAssign*)child); break;
            case NodeType::Return: compile_return((NodeReturn*)child); break;
            case NodeType::If: compile_if((NodeIf*)child); break;
            case NodeType::For: compile_for((NodeFor*)child); break;
            case NodeType::While: compile_while((NodeWhile*)child); break;
        }
    }
    Logger::end_scope();
}

void Code::compile_let(NodeLet *node)
{
    Logger::log(node->get_name().debug_info, "Compile let");

    NodeExpression *value = node->get_value();
    Symbol symb;
    int size;

    if (value != nullptr)
    {
        compile_rexpression(value);
        node->symbolize();
        symb = node->get_symb();

        size = DataType::find_size(symb.type);
        write_byte(BC_STORE_LOCAL_X);
        write_int(size);
        write_byte(symb.location);
    }
    else
        node->symbolize();
}

void Code::compile_assign(NodeAssign *node)
{
    Logger::log(node->get_left()->get_data()->token.debug_info, "Compile assign");

    NodeExpression *left = node->get_left();
    NodeExpression *right = node->get_right();

    if (right != nullptr)
    {
        compile_rexpression(right);
        compile_lexpression(left);
        
        int size = DataType::find_size(right->get_data_type());
        write_byte(BC_ASSIGN_REF_X);
        write_int(size);
    }
    else
    {
        compile_rexpression(left);
        
        int size = DataType::find_size(left->get_data_type());
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

void Code::compile_for(NodeFor *node)
{
    string start = gen_label();
    string end = gen_label();

    // Assign initial value
    NodeExpression *from = node->get_from();
    compile_rexpression(from);
    compile_lexpression(node->get_left());
    
    int size = DataType::find_size(from->get_data_type());
    write_byte(BC_ASSIGN_REF_X);
    write_int(size);

    // Check to see if within loop
    assign_label(start);
    compile_rexpression(node->get_to());
    compile_rexpression(node->get_left());
    write_byte(BC_MORE_THAN_INT_INT);
    write_byte(BC_JUMP_IF_NOT);
    write_label(end);

    compile_block((NodeCodeBlock*)node);

    // Increment by 1
    compile_rexpression(node->get_left());
    write_byte(BC_PUSH_4);
    write_int(1);
    write_byte(BC_ADD_INT_INT);
    compile_lexpression(node->get_left());
    write_byte(BC_ASSIGN_REF_X);
    write_int(size);

    // Jump to the start
    write_byte(BC_JUMP);
    write_label(start);
    assign_label(end);
}

void Code::compile_while(NodeWhile *node)
{
    string start = gen_label();
    string end = gen_label();

    // Exit loop if condition is not met
    assign_label(start);
    compile_rexpression(node->get_condition());
    write_byte(BC_JUMP_IF_NOT);
    write_label(end);

    // Compile main code
    compile_block((NodeCodeBlock*)node);
    
    // Jump to the start of the loop
    write_byte(BC_JUMP);
    write_label(start);
    assign_label(end);
}
