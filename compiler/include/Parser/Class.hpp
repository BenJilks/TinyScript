#pragma once
#include "Node.hpp"

namespace TinyScript
{

    class NodeClass : NodeBlock
    {
    public:
        NodeClass(Node *parent) :
            NodeBlock(parent) {}
        
        virtual NodeType get_type() { return NodeType::Class; }
        virtual void parse(Tokenizer &tk);

    private:
        void parse_attr(Tokenizer &tk);

        Token name;

    };

}
