#pragma once
#include "Node.hpp"

namespace TinyScript
{

    class NodeClass : public NodeBlock
    {
    public:
        NodeClass(Node *parent) :
            NodeBlock(parent, true) {}
        
        virtual NodeType get_type() { return NodeType::Class; }
        virtual void parse(Tokenizer &tk);

    private:
        void parse_attr(Tokenizer &tk);

        Token name;
        DataConstruct *construct;

    };

}
