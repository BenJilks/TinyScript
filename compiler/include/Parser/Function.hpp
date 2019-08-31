#pragma once
#include "Node.hpp"
#include "Expression.hpp"
#include "DataType.hpp"

namespace TinyScript
{

    class NodeCodeBlock : public NodeBlock
    {
    public:
        NodeCodeBlock(Node *parent, bool is_allocator = false) :
            NodeBlock(parent, is_allocator) {}
        
        // Copy constructor
        NodeCodeBlock(NodeCodeBlock *other) :
            NodeBlock(other) {}

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
        virtual Node *copy(Node *parent);
        void symbolize();
        
        inline Token get_name() const { return name; }
        inline Symbol get_symb() const { return symb; }
        inline NodeExpression *get_value() const { return value; }
        
    private:
        Token name;
        Symbol symb;
        NodeExpression *value;

        NodeDataType *static_type;
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
        virtual Node *copy(Node *parent);
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
        virtual Node *copy(Node *parent);
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
        virtual Node *copy(Node *parent);
        inline NodeExpression *get_condition() const { return condition; }

    private:
        NodeExpression *condition;

    };

    class NodeFor : public NodeCodeBlock
    {
    public:
        NodeFor(Node *parent) :
            NodeCodeBlock(parent) {}
        
        ~NodeFor();

        virtual NodeType get_type() { return NodeType::For; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);

        inline NodeExpression *get_left() const { return left; }
        inline NodeExpression *get_from() const { return from; }
        inline NodeExpression *get_to() const { return to; }

    private:
        NodeExpression *left;
        NodeExpression *from, *to;

    };

    class NodeWhile : public NodeCodeBlock
    {
    public:
        NodeWhile(Node *parent) :
            NodeCodeBlock(parent) {}
        
        ~NodeWhile();

        virtual NodeType get_type() { return NodeType::While; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);

        inline NodeExpression *get_condition() const { return condition; }

    private:
        NodeExpression *condition;

    };

    class NodeFunction : public NodeCodeBlock
    {
    public:
        NodeFunction(Node *parent) :
            NodeCodeBlock(parent, true),
            is_compiled_flag(false),
            arg_size(0) {}
        
        // Copy constructor
        NodeFunction(NodeFunction *other) :
            NodeCodeBlock(other),
            name(other->name),
            symb(other->symb),
            arg_size(other->arg_size),
            is_template_flag(other->is_template_flag) {}

        ~NodeFunction();

        virtual NodeType get_type() { return NodeType::Function; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        inline Token get_name() const { return name; }
        inline Symbol get_symb() const { return symb; }
        inline int get_arg_size() const { return arg_size; }
        void register_func();

        const Symbol &implement(vector<DataType> params);
        inline bool is_template() const { return is_template_flag; }
        inline bool is_compiled() const { return is_compiled_flag; }
        inline void set_compiled() { is_compiled_flag = true; }
    
    private:
        void parse_params(Tokenizer &tk);
        void parse_return_type(Tokenizer &tk);

        Token name;
        Symbol symb;
        vector<std::pair<Token, NodeDataType*>> params;
        NodeDataType *return_type_node;

        int arg_size;
        bool is_template_flag;
        bool is_compiled_flag;

    };

}
