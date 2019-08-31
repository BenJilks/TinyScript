#include <iostream>
#include <memory.h>
#include "Parser/Program.hpp"
#include "CodeGen/TinyVMCode.hpp"
#include "flags.h"
extern "C"
{
#include "vm.h"
#include "std.h"
#include "bytecode.h"
}
using namespace TinyScript;

void import_std(NodeProgram &prog)
{
    prog.add_src("../std/io.tiny");
}

int run_bin(string path)
{
    FILE *file = fopen(path.c_str(), "rb");
    fseek(file, 0L, SEEK_END);
    int len = ftell(file);
    rewind(file);

    int main_func;
    char *code = (char*)malloc(len - sizeof(int));
    fread(&main_func, 1, sizeof(int), file);
    fread(code, 1, len - sizeof(int), file);

    vm_init();
    register_std();
    vm_run(code, main_func, NULL);
    free(code);

    return 0;
}

int main(int argc, char *argv[])
{
    TinyVM::Code code;
    NodeProgram prog;
    string output = "";
    string bin = "";
    //import_std(prog);

    // Include all files parsed into compiler
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-o" || arg == "--output")
        {
            if (i >= argc - 1)
                Logger::link_error("Expected output file");
            else
                output = argv[++i];
        }
        else if (arg == "-b" || arg == "--bin")
        {
            if (i >= argc - 1)
                Logger::link_error("Expected bin file");
            else
                return run_bin(argv[++i]);
        }
        else
        {
            prog.add_src(argv[i]);
        }
    }

    prog.parse();
    code.compile_program(prog);
    vector<char> bytecode = code.link();
    int main_func = code.find_funcion("test.main");

    char *raw_code = (char*)malloc(bytecode.size());
    memcpy(raw_code, &bytecode[0], bytecode.size());

#if DEBUG_ASSEMBLY
        printf("\nDisassembly: ");
        disassemble(raw_code, bytecode.size());
        printf("\n");
#endif

    if (!Logger::has_error())
    {
        if (output != "")
        {
            FILE *file = fopen(output.c_str(), "wb");
            fwrite(&main_func, 1, sizeof(int), file);
            fwrite(raw_code, 1, bytecode.size(), file);
            fclose(file);
            return 0;
        }

        vm_init();
        register_std();
        vm_run(raw_code, main_func, NULL);
    }

    free(raw_code);
}
