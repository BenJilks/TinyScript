#include <iostream>
#include "Module.hpp"
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
    ModuleLibrary library;
    library.add_external(Std::load_io());
    library.load_module("../test_scripts/test");
    CodeGen code = library.compile();

    if (!Logger::has_error())
    {
        vector<char> bytecode = code.link();
        vm_init();
        register_std();
        vm_run(&bytecode[0], code.find_func_loc("main"), NULL);
    }
}
