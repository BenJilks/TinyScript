#pragma once
#include "Node.hpp"
#include "DataType.hpp"

namespace TinyScript
{

    class NodeClass : public NodeBlock
    {
    public:
        NodeClass(Node *parent) :
            NodeBlock(parent, true) {}

        ~NodeClass();

        virtual NodeType get_type() { return NodeType::Class; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        void register_class();

    private:
        void parse_attr(Tokenizer &tk);

        Token name;
        DataConstruct *construct;
        vector<std::pair<NodeDataType*, Token>> attrs;

    };

}
