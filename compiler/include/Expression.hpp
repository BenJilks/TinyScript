#pragma once
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include "CodeGen.hpp"
#include <functional>
using std::function;

#define NODE_OPERATION  0b100000
#define NODE_CALL       0b010000
#define NODE_INDEX      0b001000
#define NODE_CAST       0b000100
#define NODE_ARRAY      0b000010
#define NODE_TEMP_REF   0b000001

namespace TinyScript
{

    struct ExpNode
    {
        ExpNode *left;
        ExpNode *right;
        DataType type;
        int flags;

        Token token;
        vector<ExpNode*> args;
        Symbol symb;
    };

    class Expression
    {
    public:
        Expression(Tokenizer *tk);
        DataType parse_type(SymbolTable &table);
        ExpNode *parse(SymbolTable &table);
        void free_node(ExpNode *node);

        static void compile_rvalue(ExpNode *node, CodeGen &code, SymbolTable &table);
        static void compile_lvalue(ExpNode *node, CodeGen &code, SymbolTable &table);
        static bool is_static_lvalue(ExpNode *node);
        static Symbol find_lvalue_location(ExpNode *node);
        static void assign_symb(Symbol symb, CodeGen &code);

    private:
        Tokenizer *tk;

        // Compiler functions
        ExpNode *parse_type_name(SymbolTable &table, ExpNode *node);
        ExpNode *parse_type_size(SymbolTable &table, ExpNode *node);
        ExpNode *parse_in(SymbolTable &table, ExpNode *node);
        ExpNode *parse_cast(SymbolTable &table, ExpNode *node);
        ExpNode *parse_indies(SymbolTable &table, ExpNode *node);
        ExpNode *parse_transforms(SymbolTable &table, ExpNode *node);

        const Symbol &find_alternet_overload(SymbolTable &table, ExpNode *node, 
            Token name, vector<DataType> params);
        DataType parse_array_type(SymbolTable &table, DataType of);
        ExpNode *parse_term(SymbolTable &table);
        void parse_array(SymbolTable &table, ExpNode *node);
        void parse_ref(SymbolTable &table, ExpNode *node);
        void parse_copy(SymbolTable &table, ExpNode *node);
        void parse_negate(SymbolTable &table, ExpNode *node);
        void parse_name(SymbolTable &table, ExpNode *node);
        void parse_args(SymbolTable &table, ExpNode *node);
        ExpNode *parse_sub_expression(SymbolTable &table, ExpNode *node);

        template<typename Func>
        ExpNode *parse_operation(SymbolTable &table, Func next_term, 
            vector<TokenType> ops);
        
        DataType parse_operation_type(ExpNode *left, ExpNode *right, Token op);

    };

}
