#pragma once
#include "Node.hpp"
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include <functional>
using std::function;

#define NODE_OPERATION  0b10000000
#define NODE_CALL       0b01000000
#define NODE_INDEX      0b00100000
#define NODE_CAST       0b00010000
#define NODE_ARRAY      0b00001000
#define NODE_TEMP_REF   0b00000100
#define NODE_IN         0b00000010
#define NODE_ARGS_LIST  0b00000001

namespace TinyScript
{

    struct ExpDataNode
    {
        ExpDataNode *left;
        ExpDataNode *right;
        DataType type;
        int flags;

        Token token;
        vector<ExpDataNode*> args;
        Symbol symb;
    };

    class NodeExpression : public Node
    {
    public:
        NodeExpression(Node *parent) :
            Node(parent) {}

        virtual NodeType get_type() { return NodeType::Expression; }
        virtual void parse(Tokenizer &tk);
        virtual Node *copy(Node *parent);
        void symbolize();

        inline DataType get_data_type() const { return exp->type; }
        inline ExpDataNode *get_data() const { return exp; }
        bool is_static_value() const;
        int get_int_value() const;
        
        ~NodeExpression() { free_node(exp); }

    private:
        // Compiler functions
        ExpDataNode *parse_type_name(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_type_size(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_array_size(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_in(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_cast(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_indies(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_transforms(Tokenizer &tk, ExpDataNode *node);

        DataType parse_array_type(Tokenizer &tk, DataType of);
        ExpDataNode *parse_term(Tokenizer &tk);
        void parse_array(Tokenizer &tk, ExpDataNode *node);
        void parse_ref(Tokenizer &tk, ExpDataNode *node);
        void parse_copy(Tokenizer &tk, ExpDataNode *node);
        void parse_negate(Tokenizer &tk, ExpDataNode *node);
        void parse_name(Tokenizer &tk, ExpDataNode *node);
        void parse_args(Tokenizer &tk, ExpDataNode *node);
        ExpDataNode *parse_sub_expression(Tokenizer &tk, ExpDataNode *node);

        template<typename Func>
        ExpDataNode *parse_operation(Tokenizer &tk, Func next_term, 
            vector<TokenType> ops);
        
        DataType parse_operation_type(ExpDataNode *left, ExpDataNode *right, Token op);
        ExpDataNode *parse_expression(Tokenizer &tk);
        void free_node(ExpDataNode *node);
        void symbolize_node(ExpDataNode *node);

        // Symbolizer
        void symbolize_module_attr(ExpDataNode *node, const Symbol &left_symb);
        void symbolize_ref(ExpDataNode *node);
        void symbolize_copy(ExpDataNode *node);
        void symbolize_typesize(ExpDataNode *node);
        void symbolize_typename(ExpDataNode *node);
        void symbolize_arraysize(ExpDataNode *node);
        void symbolize_array(ExpDataNode *node);
        bool symbolize_special(ExpDataNode *node);
        
        // Expression data
        ExpDataNode *exp;
    };

}
