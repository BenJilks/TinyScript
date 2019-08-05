#pragma once
#include <string>
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include "CodeGen.hpp"
#include "Expression.hpp"
using std::string;

namespace TinyScript
{

    class Function
    {
    public:
        Function(string name, const DebugInfo start, const DebugInfo return_code, Symbol func_symb,
            vector<Symbol> params, DataType return_type, Tokenizer *tk);
        
        inline const Symbol &get_symbol() const { return func_symb; }
        void add_prefix(string prefix);
        CodeGen compile(SymbolTable &table);

    private:
        string name;
        Symbol func_symb;
        const DebugInfo start;
        const DebugInfo return_code;
        const vector<Symbol> params;
        const DataType return_type;
        int return_size, arg_size;
        Tokenizer *tk;
        Expression exp;
        CodeGen code;

        void compile_block(SymbolTable &table);
        void compile_statement(SymbolTable &table);
        
        void compile_let(SymbolTable &table);
        void assign_inline(SymbolTable &table, Token name);
        void compile_return(SymbolTable &table);
        void compile_if(SymbolTable &table);
        void compile_expression(SymbolTable &table);
        void parse_static_assign(ExpNode *left, ExpNode *right, SymbolTable &table);
        void parse_dynamic_assign(ExpNode *left, ExpNode *right, SymbolTable &table);
        void parse_assign(ExpNode *left, SymbolTable &table);

    };

}
