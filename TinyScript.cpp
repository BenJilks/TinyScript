#include <iostream>
#include "Parser/Module.hpp"
#include "CodeGen/TinyVMCode.hpp"
#include "Std.hpp"
extern "C"
{
#include "vm.h"
#include "std.h"
#include "bytecode.h"
}
using namespace TinyScript;

int main()
{
    Tokenizer tk("../test_scripts/test.tiny");
    TinyVM::Code code;

    NodeModule mod;
    mod.parse(tk);
    code.compile_module(&mod);

    if (!Logger::has_error())
    {
        vector<char> bytecode = code.link();
        vm_init();
        register_std();

        int i;
        vm_run(&bytecode[0], code.find_funcion("main"), (char*)&i);
        printf("%i\n", i);
    }
}
