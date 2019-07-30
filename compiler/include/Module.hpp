#pragma once
#include <string>
#include "Tokenizer.hpp"
#include "Function.hpp"
#include "Symbol.hpp"
#include "Expression.hpp"
using std::string;

namespace TinyScript
{

    class ModuleLibrary;
    class Module
    {
    public:
        Module(string name, Tokenizer tk);
        Module(string name);
        Module(Module &&other);
        inline string get_name() const { return name; }
        inline bool is_external() const { return is_external_flag; }

        void gen_header(ModuleLibrary *library);
        void add_external_func(string name, DataType type, vector<DataType> params);
        CodeGen compile();

    private:
        const string name;
        vector<Function> funcs;
        map<Module*, vector<string>> imports;
        SymbolTable table;
        Tokenizer tk;
        Expression exp;
        CodeGen code;
        bool is_external_flag;

        CodeGen compile_external();
        void parse_import_from(ModuleLibrary *library);
        void parse_class();
        void parse_func();
        vector<Symbol> parse_params();
        DataType parse_return_type();

    };

    class ModuleLibrary
    {
    public:
        ModuleLibrary() {}
        void add_external(Module *mod);
        Module *load_module(string name);
        Module *find_module(string name);
        CodeGen compile();

        ~ModuleLibrary();

    private:
        vector<Module*> modules;

    };

}
