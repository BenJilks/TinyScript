#include "CodeGen/TinyVMCode.hpp"
using namespace TinyScript::TinyVM;

vector<char> Code::link()
{
    return code;
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
