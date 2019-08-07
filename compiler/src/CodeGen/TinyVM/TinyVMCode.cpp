#include "CodeGen/TinyVMCode.hpp"
#include "memory.h"
#include "flags.h"
#include <algorithm>
using namespace TinyScript::TinyVM;

vector<char> Code::link()
{
    vector<char> code_out = code;
    for (auto label : labels)
    {
        auto label_info = label.second;
        vector<int> addrs = std::get<0>(label_info);
        int location = std::get<1>(label_info);
        
        if (location == -1)
            printf("\33[1;31mLink Error: Label '%s' was referenced but never assigned\n\33[0m", 
                label.first.c_str());

        for (int addr : addrs)
            memcpy(&code_out[addr], &location, sizeof(int));
    }

    vector<char> header;
    header.push_back(externals.size());
    for (Symbol symb : externals)
    {
        string name = Symbol::printout(symb);
        int id = symb.location;

        int start = header.size();
        header.resize(start + 4 + 1 + name.length());
        memcpy(&header[start], &id, sizeof(int));
        memcpy(&header[start + 5], name.c_str(), name.length());
        header[start + 4] = name.length();
    }

    vector<char> out;
    out.insert(out.end(), header.begin(), header.end());
    out.insert(out.end(), code_out.begin(), code_out.end());

    return out;
}

void Code::write_byte(char b)
{
    code.push_back(b);
}

void Code::write_int(int i)
{
    for (int j = 0; j < sizeof(int); j++)
        code.push_back((i >> j*8) & 0xFF);
}

void Code::write_float(float f)
{
    int i = *(int*)&f;
    write_int(i);
}

void Code::write_string(string str)
{
    write_byte(str.length() + 1);
    for (char c : str)
        write_byte(c);
    write_byte('\0');
}

void Code::write_label(string label)
{
#if DEBUG_LINK
    printf("Call %s\n", label.c_str());
#endif

    if (labels.find(label) == labels.end())
        labels[label] = std::make_tuple(vector<int>(), -1);
    std::get<0>(labels[label]).push_back(code.size());
    write_int(0);
}

void Code::assign_label(string label)
{
#if DEBUG_LINK
    printf("Assign %s\n", label.c_str());
#endif

    int location = code.size();
    if (labels.find(label) == labels.end())
        labels[label] = std::make_tuple(vector<int>(), location);
    else
        std::get<1>(labels[label]) = location;
}

int Code::find_funcion(string name)
{
    if (labels.find(name) == labels.end())
    {
        printf("\33[1;31mLink Error: Could not find function '%s'\33[0m\n", 
            name.c_str());
        return -1;
    }

    return std::get<1>(labels[name]);
}

string Code::gen_label()
{
    int id;
    do
    {
        id = random();
    } while(std::find(
        used_labels.begin(), used_labels.end(), id) 
        != used_labels.end());

    used_labels.push_back(id);
    return "$(" + std::to_string(id) + ")";
}
