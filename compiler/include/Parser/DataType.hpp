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
            Node(parent), is_auto_flag(false) {}

        virtual void parse(Tokenizer &tk);
        virtual NodeType get_type() { return NodeType::DataType; }
        virtual Node *copy(Node *parent);

        DataType compile();
        inline bool is_auto() const { return is_auto_flag; }
    
    private:
        Token type_name;
        vector<DataTypeModifier> modifiers;
        bool is_auto_flag;

        void parse_array(Tokenizer &tk);
        DataType compile_ref(DataType base);
        DataType compile_array(DataType base, DataTypeModifier mod);

    };

}
