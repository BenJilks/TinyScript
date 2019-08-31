#include "CodeGen/TinyVMCode.hpp"
using namespace TinyScript::TinyVM;

void Code::compile_import(NodeImport *node)
{
    node->symbolize();
}

void Code::compile_import_from(NodeImportFrom *node)
{
    node->symbolize();
}

void Code::compile_external(NodeExtern *node)
{
    Symbol symb = node->get_symb();

    // If the external has already been added, don't add it
    for (Symbol external : externals)
        if (external.location == symb.location)
            return;

    externals.push_back(symb);
}

void Code::compile_module(NodeModule *node)
{
    Token name = node->get_name();
    Logger::log(name.debug_info, "Compiling module '" + name.data + "'");
    Logger::start_scope();

    for (int i = 0; i < node->get_child_size(); i++)
    {
        Node *child = (*node)[i];
        switch (child->get_type())
        {
            case NodeType::Function: compile_function((NodeFunction*)child); break;
            case NodeType::ImportFrom: compile_import_from((NodeImportFrom*)child); break;
            case NodeType::Import: compile_import((NodeImport*)child); break;
            case NodeType::Extern: compile_external((NodeExtern*)child); break;
            case NodeType::Class: compile_class((NodeClass*)child); break;
        }
    }

    Logger::end_scope();
}

void Code::pre_proccess_program(NodeProgram &node)
{
    Logger::log({}, "Preprossing program");
    Logger::start_scope();

    for (int i = 0; i < node.get_child_size(); i++)
    {
        NodeModule *mod = (NodeModule*)node[i];
        mod->register_types();
        mod->register_functions();
    }

    Logger::end_scope();
}

void Code::compile_program(NodeProgram &node)
{
    Logger::log({}, "Compiling program");
    Logger::start_scope();
    pre_proccess_program(node);

    bool is_compiled = false;
    while (is_compiled == false)
    {
        for (int i = 0; i < node.get_child_size(); i++)
        {
            NodeModule *mod = (NodeModule*)node[i];
            if (!mod->is_compiled())
                compile_module(mod);
            mod->flag_compiled();
        }

        is_compiled = true;
        for (int i = 0; i < node.get_child_size(); i++)
        {
            NodeModule *mod = (NodeModule*)node[i];
            if (!mod->is_compiled())
                is_compiled = false;
        }
    }
    Logger::end_scope();
}
