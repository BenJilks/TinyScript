#include "Compiler.hpp"
#include "VM.h"
#include "IO.h"
#include "Array.h"

#include <iostream>

int Interpreter()
{
    printf("TinyScript interpreter, V0.1\n");

    size_t line_size = 1024;
    char *line = (char*)malloc(line_size);
    while (true)
    {
        printf(">> ");
        getline(&line, &line_size, stdin);
        line[strlen(line)-1] = '\0';

        if (!strcmp(line, "exit"))
            break;
    }
    free(line);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return Interpreter();

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
        RegisterString();
        RegisterArray();
        LoadProgram(&code[0], code.size());
    }
}
