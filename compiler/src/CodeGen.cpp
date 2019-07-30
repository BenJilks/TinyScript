#include "CodeGen.hpp"
#include <utility>
#include <iostream>
#include <memory.h>
using namespace TinyScript;

void CodeGen::append(CodeGen other)
{
    for (auto call : other.calls)
        calls.push_back(std::make_pair(call.first, 
            call.second + code.size()));

    for (auto func : other.funcs)
        funcs.push_back(std::make_pair(func.first, 
            func.second + code.size()));
    
    for (auto label_ref : other.label_refs)
        label_refs.push_back(std::make_pair(label_ref.first, 
            label_ref.second + code.size()));

    for (auto label : other.labels)
        labels[label.first] = label.second + code.size();

    externals.insert(externals.end(), other.externals.begin(), other.externals.end());
    code.insert(code.end(), other.code.begin(), other.code.end());
}

vector<char> CodeGen::link()
{
    vector<char> out_code = code;
    CodeGen header;

    header.write_byte(externals.size());
    for (Symbol external : externals)
    {
        string id = Symbol::printout(external);
        header.write_int(external.location);
        header.write_byte(id.length());
        for (char c : id)
            header.write_byte(c);
    }

    for (auto call : calls)
    {
        Symbol a = call.first;
        bool found = false;

        for (auto func : funcs)
        {
            Symbol b = func.first;
            if (a.name == b.name)
            {
                memcpy(&out_code[call.second], &func.second, sizeof(int));
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cout << "Link Error: Undefined function '" << 
                a.name << "'" << std::endl;
        }
    }

    for (auto label_ref : label_refs)
    {
        if (labels.find(label_ref.first) == labels.end())
        {
            std::cout << "Link Error: Lable not defined" << std::endl;
            continue;
        }

        memcpy(&out_code[label_ref.second], &labels[label_ref.first], sizeof(int));
    }

    vector<char> out;
    out.insert(out.end(), header.code.begin(), header.code.end());
    out.insert(out.end(), out_code.begin(), out_code.end());
    return out;
}

int CodeGen::find_func_loc(string name)
{
    for (auto func : funcs)
        if (func.first.name == name)
            return func.second;
    return -1;
}

void CodeGen::write_byte(char b)
{
    code.push_back(b);
}

void CodeGen::write_int(int i)
{
    for (int j = 0; j < sizeof(int); j++)
        code.push_back((i >> j*8) & 0xFF);
}

void CodeGen::write_float(float f)
{
    int i = *(int*)&f;
    write_int(i);
}

void CodeGen::write_string(string str)
{
    write_byte(str.length() + 1);
    for (char c : str)
        write_byte(c);
    write_byte('\0');
}

void CodeGen::write_call_loc(Symbol symb)
{
    int addr = code.size();
    write_int(0);
    calls.push_back(std::make_pair(symb, addr));
}

void CodeGen::register_func(Symbol func)
{
    funcs.push_back(std::make_pair(func, code.size()));
}

void CodeGen::write_label(int label)
{
    label_refs.push_back(std::make_pair(label, code.size()));
    write_int(0);
}

int CodeGen::create_label()
{
    return rand();
}

void CodeGen::register_label(int label)
{
    labels[label] = code.size();
}

void CodeGen::register_external(Symbol &external)
{
    int largest = -1;
    for (Symbol e : externals)
    {
        if (e.location > largest)
            largest = e.location;
    }

    external.location = rand();
    externals.push_back(external);
}

void CodeGen::insert(int location, char *data, int size)
{
    memcpy(&code[location], data, size);
}
