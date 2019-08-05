#pragma once
#include "Node.hpp"

namespace TinyScript
{

    class NodeModule : public NodeBlock
    {
    public:
        NodeModule(Node *parent = nullptr) :
            NodeBlock(parent) {}
        virtual NodeType get_type() { return NodeType::Module; }
        virtual void parse(Tokenizer &tk);

    };

}
