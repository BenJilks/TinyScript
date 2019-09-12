#pragma once
#include "Parser/Expression.hpp"
#include "Parser/Function.hpp"
#include "Parser/Class.hpp"
#include "Parser/Module.hpp"
#include "Parser/Program.hpp"
#include <tuple>
using std::tuple;

namespace TinyScript::TinyVM
{

    class Code
    {
    public:
        Code() {}

        vector<char> link();

        // Code gen functions
        void write_byte(char b);
        void write_int(int i);
        void write_float(float f);
        void write_string(string str);
        void write_label(string label);
        void assign_label(string label);
        int find_funcion(string name);
        string gen_label();

        // Compile nodes
        void compile_rexpression(NodeExpression *node);
        void compile_lexpression(NodeExpression *node);
        void compile_function(NodeFunction *node);
        void compile_block(NodeBlock *node);
        void compile_let(NodeLet *node);
        void compile_assign(NodeAssign *node);
        void compile_return(NodeReturn *node);
        void compile_if(NodeIf *node);
        void compile_for(NodeFor *node);
        void compile_while(NodeWhile *node);
        void compile_import(NodeImport *node);
        void compile_import_from(NodeImportFrom *node);
        void compile_external(NodeExtern *node);
        void compile_class(NodeClass *node);
        void compile_module(NodeModule *node);
        void compile_program(NodeProgram &node);

    private:

        // Expression
        Symbol find_lvalue_location(ExpDataNode *node);
        bool is_static_lvalue(ExpDataNode *node);
        void compile_operation(Token op, DataType ltype, DataType rtype);
        void compile_call(const Symbol &symb, ExpDataNode *node);
        void compile_rname(ExpDataNode *node);
        void compile_ref(ExpDataNode *node);
        void compile_copy(ExpDataNode *node);
        void compile_array(ExpDataNode *node);
        void compile_cast(ExpDataNode *node);
        void compile_rterm(ExpDataNode *node);
        void compile_index(ExpDataNode *node);
        void compile_rin(ExpDataNode *node);
        void compile_lname(ExpDataNode *node);
        void compile_lterm(ExpDataNode *node);
        void compile_lin(ExpDataNode *node);
        void compile_typesize(ExpDataNode *node);
        void compile_typename(ExpDataNode *node);
        void compile_arraysize(ExpDataNode *node);
        void compile_rvalue(ExpDataNode *node);
        void compile_lvalue(ExpDataNode *node);

        // Code data
        vector<char> code;
        vector<Symbol> externals;
        map<string, tuple<vector<int>, int>> labels;
        vector<int> used_labels;

    };

}
