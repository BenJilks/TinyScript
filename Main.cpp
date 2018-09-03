#include "Compiler.hpp"
#include "VM.h"
#include "Disassemble.h"
#include <iostream>
#include <fstream>

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

    Compiler *compiler = new Compiler();
    string file_path = "";
    string output_bin = "";
    bool bin_file = false;
    bool dis_mode = false;

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-o")
            output_bin = argv[++i];
        else if (arg == "-bin")
            bin_file = true;
        else if (arg == "-dis")
            dis_mode = true;
        else
            file_path = arg;
    }

    if (dis_mode)
    {
        printf("Disassemble file: %s\n", file_path.c_str());
        Disassemble(file_path.c_str());
        return 0;
    }

    if (bin_file)
    {
        ExecFile(&file_path[0]);
        return 0;
    }

    if (file_path == "")
    {
        cout << "Must give a script file" << endl;
        return -1;
    }

    bool has_error = compiler->Compile(file_path);
    if (has_error)
    {
        cout << "Could not execute code due to compiler error" << endl;
        return -1;
    }

    vector<char> code = compiler->GetDump();
    delete compiler;
    if (code.size() > 0)
    {
        if (output_bin == "")
        {
            LoadProgram(&code[0], code.size());
            return 0;
        }
        
        FILE *file = fopen(output_bin.c_str(), "wb");
        fwrite(&code[0], sizeof(char), code.size(), file);
        fclose(file);
        return 0;
    }

    cout << "Linker error" << endl;
    return -1;
}
