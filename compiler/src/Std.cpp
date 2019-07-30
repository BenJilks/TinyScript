#include "Std.hpp"
using namespace TinyScript;

Module *Std::load_io()
{
    Module *io = new Module("io");

    // func log(char ref str)
    /*
    io->add_external_func("log", 
        { PrimTypes::type_null(), 0, 0 }, 
        { { PrimTypes::type_char(), DATATYPE_REF, 0 } });
    */
    
    // func log(char ref str)
    io->add_external_func("log", 
        { PrimTypes::type_null(), 0, 0 }, 
        { { PrimTypes::type_char(), DATATYPE_REF, 0 } });

    // func log(int i)
    io->add_external_func("log", 
        { PrimTypes::type_null(), 0, 0 }, 
        { { PrimTypes::type_int(), 0, 0 } });

    // func log(float i)
    io->add_external_func("log", 
        { PrimTypes::type_null(), 0, 0 }, 
        { { PrimTypes::type_float(), 0, 0 } });

    // func log(char i)
    io->add_external_func("log", 
        { PrimTypes::type_null(), 0, 0 }, 
        { { PrimTypes::type_char(), 0, 0 } });

    // func log(bool i)
    io->add_external_func("log", 
        { PrimTypes::type_null(), 0, 0 }, 
        { { PrimTypes::type_bool(), 0, 0 } });

    return io;
}
