#pragma once
#include "Node.hpp"
#include "Expression.hpp"

namespace TinyScript
{

    class NodeCodeBlock : public NodeBlock
    {
    public:
        NodeCodeBlock(Node *parent) :
            NodeBlock(parent) {}

    protected:
        void parse_statement(Tokenizer &tk);
        void parse_block(Tokenizer &tk);
    
    };

    class NodeLet : public Node
    {
    public:
        NodeLet(Node *parent) :
            Node(parent) {}
        
        ~NodeLet();

        virtual NodeType get_type() { return NodeType::Let; }
        virtual void parse(Tokenizer &tk);
        inline Token get_name() const { return name; }
        inline Symbol get_symb() const { return symb; }
        inline NodeExpression *get_value() const { return value; }
        
    private:
        Token name;
        Symbol symb;
        NodeExpression *value;
        
    };

    class NodeAssign : public Node
    {
    public:
        NodeAssign(Node *parent) :
            Node(parent) {}
        
        ~NodeAssign();
        
        virtual NodeType get_type() { return NodeType::Assign; }
        virtual void parse(Tokenizer &tk);
        inline NodeExpression *get_left() const { return left; }
        inline NodeExpression *get_right() const { return right; }

    private:
        NodeExpression *left, *right;

    };

    class NodeFunction : public NodeCodeBlock
    {
    public:
        NodeFunction(Node *parent) :
            NodeCodeBlock(parent) {}

        virtual NodeType get_type() { return NodeType::Function; }
        virtual void parse(Tokenizer &tk);
        inline Token get_name() const { return name; }
    
    private:
        vector<DataType> parse_params(Tokenizer &tk);
        DataType parse_return_type(Tokenizer &tk);

        Token name;

    };

}
