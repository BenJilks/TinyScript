#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "Logger.hpp"
using std::string;
using std::vector;
using std::map;

namespace TinyScript
{

    enum class TokenType
    {
        // Datatypes
        Name,
        Int,
        Float,
        Bool,
        Char,
        String,
        Auto,

        // Operations
        Add,
        Subtract,
        Multiply,
        Divide,
        MoreThan,
        LessThan,
        MoreThanEquals,
        LessThanEquals,
        Equals,

        // Memory management
        Assign,
        Ref,
        Array,
        Copy,

        // Scope definers
        OpenBlock,
        CloseBlock,
        OpenArg,
        CloseArg,
        OpenIndex,
        CloseIndex,
        Next,
        Gives,
        Of,
        As,
        In,

        // Key words
        Func,
        Class,
        Let,
        Return,
        If,
        For,
        While,
        From,
        To,
        Import,
        Extern,

        // Compiler functions
        TypeName,
        TypeSize,
        ArraySize,
        TypeOf,

        // Misc
        Eof,
    };

    struct Token
    {
        string data;
        TokenType type;
        DebugInfo debug_info;
    };

    class Tokenizer
    {

    public:
        Tokenizer() {}
        Tokenizer(string file_path);
        Tokenizer(Tokenizer &other);
        inline const Token &get_look() const { return look; };
        inline const DebugInfo &get_debug_info() const { return debug_info; }
        bool is_eof() const;
        void set_pos(const DebugInfo &pos);

        Token match(TokenType type, string name);
        Token skip_token();
        void skip_scope();

    private:
        std::ifstream file;
        string file_path;
        Token look;
        bool eof_override;

        vector<char> back_buffer;
        DebugInfo debug_info;

        Token next_token();
        Token parse_double(char c, TokenType def, map<char, TokenType> m, DebugInfo dbi);
        Token parse_name(char c, DebugInfo dbi);
        Token parse_number(char c, DebugInfo dbi);
        Token parse_char(DebugInfo dbi);
        Token parse_string(DebugInfo dbi);
        char next_escaped_char();
        char next_char();
        void skip_whitespace();
        void parse_comment();

    };

}
