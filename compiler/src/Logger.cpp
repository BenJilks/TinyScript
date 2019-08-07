#include "Logger.hpp"
#include "flags.h"
#include <iostream>
#include <sstream>
#include <fstream>
using namespace TinyScript;

static bool has_error_flag = false;
static int scope = 0;

#define RED     "\033[1;31m"
#define ORANGE  "\033[01;33m"
#define RESET   "\033[0m"

void Logger::reset()
{
    has_error_flag = false;
    scope = 0;
}

static void log_info(const DebugInfo &debug_info)
{
    std::stringstream stream;
    stream << "(" << debug_info.file_name << ") " << 
        "[" << debug_info.line_no << ":" << 
        debug_info.char_no << "]";
    
    string info = stream.str();
    std::cout << info;

    int space_len = debug_info.file_name.length() + 15 - info.length();
    for (int i = 0; i < space_len; i++)
        std::cout << " ";
}

static void log_location(const DebugInfo &debug_info)
{
    string line;
    std::ifstream file(debug_info.file_name);
    for(int i = 0; i < debug_info.line_no; ++i)
        std::getline(file, line);
    file.close();

    std::cout << line << std::endl;
    for (int i = 0; i < debug_info.char_no - 1; i++)
        std::cout << " ";
    std::cout << "^^^^\n" << std::endl;
}

void Logger::warning(const DebugInfo &debug_info, string msg)
{
    std::cout << ORANGE << "Warning: ";
    log_info(debug_info);
    std::cout << msg << std::endl;
    log_location(debug_info);
    std::cout << RESET;
}

void Logger::error(const DebugInfo &debug_info, string msg)
{
    std::cout << RED << "Error: ";
    log_info(debug_info);
    std::cout << msg << std::endl;
    log_location(debug_info);
    std::cout << RESET;
    
    has_error_flag = true;
}

bool Logger::has_error()
{
    return has_error_flag;
}

#if DEBUG_COMPILER


void Logger::start_scope()
{
    scope++;
}

void Logger::end_scope()
{
    scope--;
}

void Logger::log(const DebugInfo &debug_info, string msg)
{
    log_info(debug_info);
    for (int i = 0; i < scope; i++)
        std::cout << "  ";
    
    if (scope > 0)
        std::cout << "=> ";
    std::cout << msg << std::endl;
}

#else

void Logger::start_scope() {}
void Logger::end_scope() {}
void Logger::log(const DebugInfo &debug_info, string msg) {}

#endif
