#include "Parser/Program.hpp"
#include "Parser/Module.hpp"
using namespace TinyScript;

NodeModule *NodeProgram::find_module(string name) const
{
    for (Node *child : children)
    {
        NodeModule *mod = (NodeModule*)child;
        if (mod->get_name() == name)
            return mod;
    }

    return nullptr;
}

void NodeProgram::parse()
{
    for (string src : srcs)
    {
        int n_start = src.find_last_of('/') + 1;
        int n_end = src.find_last_of('.');
        int n_len = n_end - n_start;
        string name = src.substr(n_start, n_len);

        Tokenizer tk(src);
        Logger::log(tk.get_debug_info(), "Parsing module '" + name + "'");
        NodeModule *mod = parse_node<NodeModule>(tk);
        mod->set_name(name);
        add_child(mod);
    }
}
