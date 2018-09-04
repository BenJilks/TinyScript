#pragma once
#include "Bytecode.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include <memory.h>
using namespace std;

class CodeGen
{
public:
    CodeGen() {}
    CodeGen(vector<char> code) :
        code(code) {}

    inline void Instruction(ByteCode inst) { code.push_back((char)inst); }
    inline int CurrPC() const { return code.size(); }

    vector<char> GetCode() 
    {
        vector<char> out_code = code;
        for (auto pair : labels)
        {
            int location = label_locations[pair.first];
            for (int l : pair.second)
                memcpy(&out_code[l], &location, sizeof(int));
        }
        return out_code;
    }
    
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
        int start_index = code.size();

        // Copy label data over to this code base
        for (auto pair : other.labels)
        {
            int label = pair.first;
            if (labels.find(label) == labels.end())
                labels[label] = vector<int>();
            
            for (int l : pair.second)
                labels[label].push_back(l + code.size());
        }

        // If the label has been resolved, then copy that
        for (auto pair : other.label_locations)
            label_locations[pair.first] = pair.second + code.size();
        
        vector<char> other_code = other.code;
        code.insert(code.end(), other_code.begin(), 
            other_code.end());
    }

    void Label(int label)
    {
        if (labels.find(label) == labels.end())
            labels[label] = vector<int>();
        
        labels[label].push_back(code.size());
        Argument((int)0);
    }

    void SetLabel(int label)
    {
        label_locations[label] = code.size();
    }

    int MakeLabel()
    {
        return rand();
    }

private:
    vector<char> code;
    map<int, vector<int>> labels;
    map<int, int> label_locations;

};
