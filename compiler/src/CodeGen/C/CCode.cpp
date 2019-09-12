#include "flags.h"

#if ARC_C

#include "CodeGen/CCode.hpp"
#include <sys/stat.h>
using namespace TinyScript::C;

Code::Code(string project_dir) :
    project_dir(project_dir),
    has_file(false)
{
    // Create the C project folder
    mkdir(project_dir.c_str(), 
        S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void Code::start_file(string name)
{
    // If a file was open, close it
    if (has_file)
    {
        curr_file.close();
        curr_file_header.close();
    }

    // Open up a new source file
    curr_file = std::ofstream(project_dir + "/" + name + ".c");
    curr_file_header = std::ofstream(project_dir + "/" + name + ".h");
    has_file = true;
    scope = 0;
}

void Code::write_line(string line)
{
    if (has_file)
    {
        // Write the scope tabs and end of line tokens
        for (int i = 0; i < scope; i++)
            line = "    " + line;
        line += '\n';

        // Write the line to the file
        curr_file.write(line.c_str(), line.length());
    }
}

void Code::write_header(string line)
{
    if (has_file)
    {
        line += '\n';
        curr_file_header.write(line.c_str(), line.length());
    }
}

string Code::compile_local_type(DataType type, string name)
{
    string construct = "error";
    int ref_count = 0;
    vector<int> array_sizes;

    // Count refs and arrays
    DataType curr_type = type;
    while (true)
    {
        if (curr_type.flags & DATATYPE_REF)
        {
            // Turn all arrays into pointers
            ref_count += array_sizes.size() + 1;
            array_sizes.clear();
        }
        else if (curr_type.flags & DATATYPE_ARRAY)
            array_sizes.push_back(curr_type.array_size);
        else
            break;
        
        curr_type = *curr_type.sub_type;
    }

    // Set base type name
    if (curr_type.construct != nullptr)
    {
        if (curr_type.construct == PrimTypes::type_null())
            construct = "void";
        else
            construct = curr_type.construct->name;
    }

    // Build name string
    string out = construct;
    for (int i = 0; i < ref_count; i++)
        out += "*";
    out += " " + name;
    for (int array_size : array_sizes)
        out += "[" + std::to_string(array_size) + "]";
    return out;
}

string Code::compile_param_type(DataType type)
{
    if (type.flags & DATATYPE_REF)
        return compile_param_type(type) + "*";
    
    if (type.flags & DATATYPE_ARRAY)
    {
        return compile_param_type(type) + 
            "[" + std::to_string(type.array_size) + "]";
    }

    if (type.construct == nullptr)
        return "error";
    if (type.construct == PrimTypes::type_null())
        return "void";
    return type.construct->name;
}

#endif
