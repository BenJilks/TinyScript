#include "Tokenizer.hpp"
#include <iostream>
using namespace TinyScript;

// Define keywords
static map<string, TokenType> keywords = 
{
    std::make_pair("ref", TokenType::Ref),
    std::make_pair("array", TokenType::Array),
    std::make_pair("copy", TokenType::Copy),
    std::make_pair("as", TokenType::As),
    std::make_pair("func", TokenType::Func),
    std::make_pair("class", TokenType::Class),
    std::make_pair("let", TokenType::Let),
    std::make_pair("return", TokenType::Return),
    std::make_pair("if", TokenType::If),
    std::make_pair("from", TokenType::From),
    std::make_pair("import", TokenType::Import),
    std::make_pair("true", TokenType::Bool),
    std::make_pair("false", TokenType::Bool),
    std::make_pair("typename", TokenType::TypeName),
    std::make_pair("typesize", TokenType::TypeSize),
    std::make_pair("typeof", TokenType::TypeOf),
    std::make_pair("auto", TokenType::Auto)
};

static map<char, TokenType> sub = { std::make_pair('>', TokenType::Gives) };
static map<char, TokenType> more = { std::make_pair('=', TokenType::MoreThanEquals) };
static map<char, TokenType> less = { std::make_pair('=', TokenType::LessThanEquals) };
static map<char, TokenType> equal = { std::make_pair('=', TokenType::Equals) };

Tokenizer::Tokenizer(string file_path) :
    debug_info { 1, 0, 0 },
    file_path(file_path)
{
    // Open file stream and find the first token
    file = std::ifstream(file_path);
    eof_override = false;
    debug_info.file_name = file_path;

    if (!file.good())
    {
        Logger::error(debug_info, "Cannot open file '" + file_path + "'");
        eof_override = true;
    }

    look = next_token();
}

Tokenizer::Tokenizer(Tokenizer &other)
{
    // Open a new stream in the exact same state as other
    debug_info = other.debug_info;
    file_path = other.file_path;
    file = std::ifstream(file_path);
    file.seekg(other.file.tellg());
    look = other.look;
    eof_override = other.eof_override;
}

Token Tokenizer::match(TokenType type, string name)
{
    if (look.type != type)
    {
        Logger::error(debug_info, "Expected token '" + name + 
            "', got '" + look.data + "' instead");
    }
    
    return skip_token();
}

Token Tokenizer::skip_token()
{
    Token tk = look;
    look = next_token();
    return tk;
}

void Tokenizer::skip_scope()
{
    int scope_depth = 1;
    while (scope_depth >= 1 && !is_eof())
    {
        char c = next_char();
        switch (c)
        {
            case '{': scope_depth++; break;
            case '}': scope_depth--; break;
        }
    }

    skip_token();
}

void Tokenizer::parse_comment()
{
    char c = next_char();
    while (c != '\n')
        c = next_char();
}

Token Tokenizer::next_token()
{
    skip_whitespace();
    DebugInfo dbi = debug_info;

    while (!is_eof())
    {
        char c = next_char();
        if (c == '#')
        {
            parse_comment();
            continue;
        }

        switch (c)
        {
            case '+': return Token { string(1, c), TokenType::Add, dbi };
            case '-': return parse_double(c, TokenType::Subtract, sub, dbi);
            case '*': return Token { string(1, c), TokenType::Multiply, dbi };
            case '/': return Token { string(1, c), TokenType::Divide, dbi };
            case '=': return parse_double(c, TokenType::Assign, equal, dbi);
            case ':': return parse_double(c, TokenType::Of, equal, dbi);
            
            case '>': return parse_double(c, TokenType::MoreThan, more, dbi);
            case '<': return parse_double(c, TokenType::LessThan, more, dbi);

            case '{': return Token { string(1, c), TokenType::OpenBlock, dbi };
            case '}': return Token { string(1, c), TokenType::CloseBlock, dbi };
            case '(': return Token { string(1, c), TokenType::OpenArg, dbi };
            case ')': return Token { string(1, c), TokenType::CloseArg, dbi };
            case '[': return Token { string(1, c), TokenType::OpenIndex, dbi };
            case ']': return Token { string(1, c), TokenType::CloseIndex, dbi };
            case ',': return Token { string(1, c), TokenType::Next, dbi };
            case '.': return Token { string(1, c), TokenType::In, dbi };

            case '\'': return parse_char(dbi);
            case '"': return parse_string(dbi);
        }

        if (isalpha(c)) return parse_name(c, dbi);
        else if (isdigit(c)) return parse_number(c, dbi);

        if (c != 0)
            Logger::error(dbi, "Unkown symbol '" + string(1, c) + "'");
    }

    return Token { "EOF", TokenType::Eof };
}

Token Tokenizer::parse_double(char c, TokenType def, map<char, TokenType> m, DebugInfo dbi)
{
    char next = next_char();
    string buffer = string(1, c);

    if (m.find(next) != m.end())
        return Token { buffer + next, m[next], dbi };
    
    back_buffer.push_back(next);
    return Token { buffer, def, dbi };
}

Token Tokenizer::parse_number(char c, DebugInfo dbi)
{
    string buffer = "";
    int point_count = 0;
    while ((isdigit(c) || c == '.') && !is_eof())
    {
        if (c == '.')
            point_count++;

        buffer += c;
        c = next_char();
    }

    // if the number contains more then 1 point, 
    // then it's an invalid number
    if (point_count > 1)
        Logger::error(debug_info, "Invalid float format '" + buffer + "'");

    back_buffer.push_back(c);
    return Token { buffer, point_count == 0 ? 
        TokenType::Int : TokenType::Float, dbi };
}

Token Tokenizer::parse_name(char c, DebugInfo dbi)
{
    string buffer = "";
    while ((isalnum(c) || c == '_') && !is_eof())
    {
        buffer += c;
        c = next_char();
    }

    back_buffer.push_back(c);
    if (keywords.find(buffer) != keywords.end())
        return Token { buffer, keywords[buffer], dbi };
    return Token { buffer, TokenType::Name, dbi };
}

char Tokenizer::next_escaped_char()
{
    char c = next_char();
    if (c == '\\')
    {
        // Parse escape char
        switch (next_char())
        {
            case '0': c = '\0'; break;
            case 'n': c = '\n'; break;
            case '\\': c = '\\'; break;
        }
    }
    return c;
}

Token Tokenizer::parse_char(DebugInfo dbi)
{
    char c = next_escaped_char();
    next_char(); // skip "'"
    return Token { string(&c, 1), TokenType::Char, dbi };
}

Token Tokenizer::parse_string(DebugInfo dbi)
{
    string buffer = "";
    char c;
    while ((c = next_escaped_char()) != '"')
        buffer += c;
    return Token { buffer, TokenType::String, dbi };
}

// Go to the next non whitespace char
void Tokenizer::skip_whitespace()
{
    char c;
    while (isspace(c = next_char()) && !is_eof())
        continue;
    back_buffer.push_back(c);
}

// Fetch the next char to be parsed
char Tokenizer::next_char()
{
    char c;
    if (back_buffer.size() > 0)
    {
        c = back_buffer.back();
        back_buffer.pop_back();
        return c;
    }

    file >> std::noskipws >> c;
    debug_info.char_no++;
    debug_info.file_pos++;
    if (c == '\n')
    {
        debug_info.line_no++;
        debug_info.char_no = 0;
    }
    return c;
}

// Return if the end of the stream as been reached
bool Tokenizer::is_eof() const
{
    return file.eof() || eof_override;
}

void Tokenizer::set_pos(const DebugInfo &pos)
{
    file.close();
    file = std::ifstream(file_path);

    debug_info = pos;
    file.seekg(pos.file_pos - 1);
    back_buffer.clear();
}
