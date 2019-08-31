#pragma once
#include "Node.hpp"

namespace TinyScript
{

    struct DataTypeModifier
    {
        Token name;
        int size;
    };

    class NodeDataType : public Node
    {
    public:
        NodeDataType(Node *parent) :
            Node(parent) {}
        
        virtual void parse(Tokenizer &tk);
        virtual NodeType get_type() { return NodeType::DataType; }
        virtual Node *copy(Node *parent);

        DataType compile();
    
    private:
        Token type_name;
        vector<DataTypeModifier> modifiers;

        void parse_array(Tokenizer &tk);
        DataType compile_ref(DataType base);
        DataType compile_array(DataType base, DataTypeModifier mod);

    };

}
