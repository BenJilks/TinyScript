#include "CodeGen/TinyVMCode.hpp"
using namespace TinyScript::TinyVM;

void Code::compile_module(NodeModule *node)
{
    for (int i = 0; i < node->get_child_size(); i++)
    {
        Node *child = (*node)[i];
        switch (child->get_type())
        {
            case NodeType::Function: compile_function((NodeFunction*)child);
        }
    }
}
