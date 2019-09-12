#include "flags.h"

#if ARC_C

#include "CodeGen/CCode.hpp"
#include "Parser/Class.hpp"
using namespace TinyScript::C;

void Code::compile_module(NodeModule *node)
{
    // Start a new file for the module
    string name = node->get_name().data;
    start_file(name);
    write_line("#include \"" + name + ".h\"\n");

    // Compile function headers
    for (Symbol symb : node->lookup_all())
    {
        if (symb.flags & SYMBOL_FUNCTION)
        {
            string header = compile_function_symbol(symb);
            write_header(header + ";");
        }
    }

    // Compile type headers
    for (DataConstruct *construct : node->find_construct())
    {
        write_header("typedef struct " + construct->name);
        write_header("{");
        for (Symbol attr : ((NodeClass*)construct->parent)->lookup_all())
            if (attr.flags & SYMBOL_LOCAL)
                write_header("    " + compile_local_type(attr.type, attr.name) + ";");
        write_header("} " + construct->name + ";");
    }

    for (int i = 0; i < node->get_child_size(); i++)
    {
        Node *child = (*node)[i];
        switch(child->get_type())
        {
            case NodeType::Function: compile_function((NodeFunction*)child); break;
        }
    }
}

void Code::compile_program(NodeProgram &node)
{
    node.pre_process();
    for (int i = 0; i < node.get_child_size(); i++)
    {
        NodeModule *mod = (NodeModule*)node[i];
        compile_module(mod);
    }
}

#endif
