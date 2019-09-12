#pragma once
#include "flags.h"

#if ARC_C

#include "Parser/Program.hpp"
#include "Parser/Module.hpp"
#include "Parser/Function.hpp"
#include <fstream>

namespace TinyScript::C
{

    struct ExpressionData
    {
        int temp_count;
        vector<string> lines;
        string value;
    };

    class Code
    {
    public:
        Code(string project_dir);

        ExpressionData compile_expression(NodeExpression *node);
        void compile_return(NodeReturn *node);
        void compile_assign(NodeAssign *node);
        void compile_let(NodeLet *node);
        void compile_block(NodeBlock *node);
        void compile_function(NodeFunction *node);
        void compile_module(NodeModule *node);
        void compile_program(NodeProgram &node);

    private:
        void start_file(string name);
        void write_line(string line);
        void write_header(string line);
        void start_scope() { scope += 1; }
        void end_scope() { scope -= 1; }

        string compile_local_type(DataType type, string name);
        string compile_param_type(DataType type);
        string compile_function_symbol(Symbol symb);
        string compile_expresion_node(ExpressionData &data, ExpDataNode *node);
        string compile_rterm(ExpressionData &data, ExpDataNode *node);
        string compile_operation(ExpressionData &data, ExpDataNode *node);
        string compile_name(ExpressionData &data, ExpDataNode *node);
        string compile_array(ExpressionData &data, ExpDataNode *node);

        string project_dir;
        std::ofstream curr_file;
        std::ofstream curr_file_header;
        int scope;
        bool has_file;

    };

}

#endif
