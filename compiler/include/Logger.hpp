#pragma once
#include <string>
using std::string;

namespace TinyScript
{
    
    struct DebugInfo
    {
        int line_no;
        int char_no;
        int file_pos;
        string file_name;
    };

    namespace Logger
    {

        void reset();
        void start_scope();
        void end_scope();

        void log(const DebugInfo &debug_info, string msg);

        void warning(const DebugInfo &debug_info, string msg);
        void error(const DebugInfo &debug_info, string msg);
        void link_error(string msg);
        bool has_error();

    }

}
