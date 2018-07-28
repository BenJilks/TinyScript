#pragma once
#include "Bytecode.hpp"
#include <vector>
#include <memory.h>
using namespace std;

class CodeGen
{
public:
    inline void Instruction(ByteCode inst) { code.push_back((char)inst); }
    inline vector<char> GetCode() const { return code; }
    inline int CurrPC() const { return code.size(); }
    
    template<typename T>
    void Argument(T arg)
    {
        char *bytes = (char*)&arg;
        code.insert(code.end(), bytes,
            bytes + sizeof(T));
    }

    void String(string str)
    {
        code.push_back((char)str.size());
        code.insert(code.end(), str.begin(), str.end());
    }

    void Append(CodeGen other)
    {
        vector<char> other_code = other.code;
        code.insert(code.end(), other_code.begin(), 
            other_code.end());
    }

    int MakeLabel()
    {
        int curr_loc = code.size();
        Argument((int)0);
        return curr_loc;
    }

    void SetLabel(int label)
    {
        int diff = code.size() - label;
        memcpy(&code[label], &diff, sizeof(int));
    }

private:
    vector<char> code;

};
