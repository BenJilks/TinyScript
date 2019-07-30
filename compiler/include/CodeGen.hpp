#pragma once
#include "Symbol.hpp"
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;

namespace TinyScript
{

    class CodeGen
    {
    public:
        CodeGen() {}
        void append(CodeGen other);
        vector<char> link();
        int find_func_loc(string name);

        void write_byte(char b);
        void write_int(int i);
        void write_float(float f);
        void write_string(string str);

        void write_label(int label);
        void write_arr_const(const vector<char> &arr) {}
        void write_str_const(const string &str) {}
        void write_call_loc(Symbol symb);

        int create_label();
        void register_func(Symbol func);
        void register_label(int label);
        void register_external(Symbol &external);

        inline int curr_location() const { return code.size(); }
        void insert(int location, char *data, int size);

        vector<char> code;
    private:
        vector<std::pair<Symbol, int>> calls;
        vector<std::pair<Symbol, int>> funcs;
        vector<Symbol> externals;

        vector<std::pair<int, int>> label_refs;
        std::map<int, int> labels;
    };

}
