#include <iostream>
#include "Parser/Program.hpp"
#include "CodeGen/TinyVMCode.hpp"
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

int main()
{
    TinyVM::Code code;
    NodeProgram prog;

    import_std(prog);
    prog.add_src("../test_scripts/maths.tiny");
    prog.add_src("../test_scripts/test.tiny");
    prog.parse();
    code.compile_program(prog);

    if (!Logger::has_error())
    {
        vector<char> bytecode = code.link();
        vm_init();
        register_std();
        vm_run(&bytecode[0], code.find_funcion("main"), NULL);
    }
}
