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

    NodeFunction func(nullptr);
    func.parse(tk);
    code.compile_function(&func);

    if (!Logger::has_error())
    {
        vector<char> bytecode = code.link();
        disassemble(&bytecode[0], bytecode.size());
        
        //vm_init();
        //register_std();
        //vm_run(&bytecode[0], code.find_func_loc("main"), NULL);
    }
}
