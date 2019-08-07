#pragma once
#include "Node.hpp"
#include "Expression.hpp"

namespace TinyScript
{

    class NodeCodeBlock : public NodeBlock
    {
    public:
        NodeCodeBlock(Node *parent, bool is_allocator = false) :
            NodeBlock(parent, is_allocator) {}

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
        void symbolize();
        
        inline Token get_name() const { return name; }
        inline Symbol get_symb() const { return symb; }
        inline NodeExpression *get_value() const { return value; }
        
    private:
        Token name;
        Symbol symb;
        NodeExpression *value;

        DataType static_type;
        bool use_static_type;
        
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

    class NodeReturn : public Node
    {
    public:
        NodeReturn(Node *parent) :
            Node(parent) {}
        
        ~NodeReturn();

        virtual NodeType get_type() { return NodeType::Return; }
        virtual void parse(Tokenizer &tk);
        inline NodeExpression *get_value() const { return value; }

    private:
        NodeExpression *value;

    };

    class NodeIf : public NodeCodeBlock
    {
    public:
        NodeIf(Node *parent) :
            NodeCodeBlock(parent) {}

        ~NodeIf();

        virtual NodeType get_type() { return NodeType::If; }
        virtual void parse(Tokenizer &tk);
        inline NodeExpression *get_condition() const { return condition; }

    private:
        NodeExpression *condition;

    };

    class NodeFunction : public NodeCodeBlock
    {
    public:
        NodeFunction(Node *parent) :
            NodeCodeBlock(parent, true) {}

        virtual NodeType get_type() { return NodeType::Function; }
        virtual void parse(Tokenizer &tk);
        inline Token get_name() const { return name; }
        inline Symbol get_symb() const { return symb; }
        inline int get_arg_size() const { return arg_size; }
    
    private:
        vector<DataType> parse_params(Tokenizer &tk);
        DataType parse_return_type(Tokenizer &tk);

        Token name;
        Symbol symb;
        int arg_size;

    };

}
