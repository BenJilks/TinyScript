#include "CodeGen/TinyVMCode.hpp"
using namespace TinyScript::TinyVM;

void Code::compile_import(NodeImport *node)
{
    node->symbolize();
}

void Code::compile_external(NodeExtern *node)
{
    externals.push_back(node->get_symb());
}

void Code::compile_module(NodeModule *node)
{
    for (int i = 0; i < node->get_child_size(); i++)
    {
        Node *child = (*node)[i];
        switch (child->get_type())
        {
            case NodeType::Function: compile_function((NodeFunction*)child); break;
            case NodeType::Import: compile_import((NodeImport*)child); break;
            case NodeType::Extern: compile_external((NodeExtern*)child); break;
        }
    }
}

void Code::compile_program(NodeProgram &node)
{
    for (int i = 0; i < node.get_child_size(); i++)
    {
        NodeModule *mod = (NodeModule*)node[i];
        compile_module(mod);
    }
}
