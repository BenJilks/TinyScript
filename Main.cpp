#include "Compiler.hpp"
#include "VM.h"
#include "IO.h"
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    Compiler compiler;
    bool has_error = compiler.Compile(argv[1]);
    if (has_error)
    {
        cout << "Could not execute code due to compiler error" << endl;
        return -1;
    }

    vector<char> code = compiler.GetDump();
    if (code.size() > 0)
    {
        RegisterIO();
        LoadProgram(&code[0], code.size());
    }
}
