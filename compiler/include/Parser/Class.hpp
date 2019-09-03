#pragma once
#include "Node.hpp"
#include "DataType.hpp"

namespace TinyScript
{

    class NodeClass : public NodeBlock
    {
    public:
        NodeClass(Node *parent) :
            NodeBlock(parent, true), 
            is_complete(false), 
            being_proccessed(false) {}

        ~NodeClass();

        virtual NodeType get_type() { return NodeType::Class; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        void register_class();
        void register_methods();

    private:
        void parse_attr(Tokenizer &tk);
        void parse_method(Tokenizer &tk);

        Token name;
        DataConstruct *construct;
        vector<std::pair<NodeDataType*, Token>> attrs;
        bool is_complete;
        bool being_proccessed;

    };

}
