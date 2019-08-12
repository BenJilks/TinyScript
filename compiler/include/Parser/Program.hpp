#pragma once
#include "Node.hpp"
#include "Module.hpp"

namespace TinyScript
{

    class NodeProgram : public NodeBlock
    {
    public:
        NodeProgram();

        inline void add_src(string path) { srcs.push_back(path); }
        inline void add_module(NodeModule *mod) { add_child(mod); }
        virtual NodeType get_type() { return NodeType::Program; }
        virtual void parse(Tokenizer &tk) {}
        virtual Node *copy(Node *parent) { return this; }
        void parse();

        NodeModule *find_module(string name) const;
        NodeModule *load_module(string name);

    private:
        vector<string> srcs;
        string cwd;

    };

}
