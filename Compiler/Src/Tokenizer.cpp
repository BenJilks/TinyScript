#include "Tokenizer.hpp"
#include <iostream>

Tokenizer::Tokenizer(string file_path):
    back_log(' ')
{
    has_error = false;
    line_number = 1;

    file = fopen(file_path.c_str(), "rb");
    if (!file)
    {
        Error("Could not open file '" + file_path + "'");
        return;
    }
    look = Next();
}

void Tokenizer::Error(string msg)
{
    // Only output the first error
    if (!has_error)
    {
        has_error = true;
        cout << "Error on line " << line_number << 
            ": " << msg << endl;
    }
}

// Throw error if a token is not what was expected
Token Tokenizer::Match(string name, TkType type)
{
    if (look.type != type)
    {
        Error("Expected token '" + name + "', got '" + 
            look.data + "' instead");
    }
    
    return NextLook();
}

// Returns the current look token, then moves into the next one
Token Tokenizer::NextLook()
{
    Token temp = look;
    look = Next();
    return temp;
}

// Fetch the next token
Token Tokenizer::Next()
{
    char c;
    while ((c = NextChar()) != EOF)
    {
        // Check char single tokens
        switch(c)
        {
            case '(': return Token(c, TkType::OpenArg);
            case ')': return Token(c, TkType::CloseArg);
            case '{': return Token(c, TkType::OpenBlock);
            case '}': return Token(c, TkType::CloseBlock);
            case '[': return Token(c, TkType::OpenIndex);
            case ']': return Token(c, TkType::CloseIndex);
            case ',': return Token(c, TkType::Next);
            case '+': return Token(c, TkType::Add);
            case '-': return Token(c, TkType::Sub);
            case '*': return Token(c, TkType::Mul);
            case '/': return Token(c, TkType::Div);
            case '>': return Token(c, TkType::GreaterThan);
            case '<': return Token(c, TkType::LessThan);
            case '.': return Token(c, TkType::Path);
            case ':': return Token(c, TkType::Of);
            case '=': return ReadEquals();
            case '"': return ReadString();
            case '\'': return ReadChar();
            case '#': SkipComment(); break;
        }
        
        // Check names and numbers
        if (isalpha(c))
            return ReadName(c);
        if (isdigit(c))
            return ReadNumber(c);
    }
    
    return Token("EOF", TkType::EndOfFile);
}

// Skip over text in a comment to ignore it
void Tokenizer::SkipComment()
{
    char c = NextChar();
    if (c == '*')
    {
        // Parse multi line comment
        char last_c = ' ';
        while (c != EOF)
        {
            if (last_c == '*' && c == '#')
                break;
            last_c = c;
            c = NextChar();
        }
    }
    else
    {
        // Parse single line comment
        while (c != '\n' && c != EOF)
            c = NextChar();
    }
}

// Check if the token ends in an extra equals sign
Token Tokenizer::ReadEquals()
{
    char c = NextChar();
    if (c == '=')
        return Token("==", TkType::Equals);
    back_log = c;
    return Token('=', TkType::Assign);
}

char Tokenizer::NextChar()
{
    // If there's a char stored in the back log, 
    // then use that char and empty the log
    if (back_log != ' ')
    {
        char c = back_log;
        back_log = ' ';
        return c;
    }

    // Get the next token from the file and increment 
    // line number if the token is a new line char
    char c = fgetc(file);
    if (c == '\\')
    {
        switch(NextChar())
        {
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case '0': c = '\0'; break;
            case '\\': c = '\\'; break;
            default: break;
        }
    }

    if (c == '\n')
        line_number++;
    return c;
}

Token Tokenizer::ReadName(char c)
{
    // Fill buffer until the char is not a letter, number or underscore
    string buffer;
    while (isalpha(c) || isdigit(c) || c == '_')
    {
        buffer += c;
        c = NextChar();
    }

    // Fill back log with last char, as this may contain a token
    back_log = c;
    return Token(buffer, FindKeywords(buffer));
}

Token Tokenizer::ReadNumber(char c)
{
    // Fill buffer until the char is not a number or a decimal place
    string buffer;
    int point_count = 0;
    while (isdigit(c) || c == '.')
    {
        // Count decimal places
        if (c == '.')
            point_count++;
        buffer += c;
        c = NextChar();
    }
    
    // Fill back log with last char, as this may contain a token
    back_log = c;
    return Token(buffer, point_count > 0 ? 
        TkType::Float : TkType::Int);
}

Token Tokenizer::ReadChar()
{
    char c = NextChar();
    NextChar(); // Skip closing "'"
    return Token(c, TkType::Char);
}

// If the buffer contains a keyword, then return it. Otherwise return name token
TkType Tokenizer::FindKeywords(string buffer)
{
    if (buffer == "func") return TkType::Function;
    if (buffer == "ret") return TkType::Return;
    if (buffer == "true") return TkType::Bool;
    if (buffer == "false") return TkType::Bool;
    if (buffer == "if") return TkType::If;
    if (buffer == "syscall") return TkType::SysCall;
    if (buffer == "for") return TkType::For;
    if (buffer == "while") return TkType::While;
    if (buffer == "to") return TkType::To;
    if (buffer == "class") return TkType::Class;
    if (buffer == "sysclass") return TkType::SysClass;
    if (buffer == "include") return TkType::Include;
    return TkType::Name;
}

Token Tokenizer::ReadString()
{
    // Fill buffer until then next '"'
    char c;
    string buffer;
    while ((c = NextChar()) != '"')
        buffer += c;
    return Token(buffer, TkType::String);
}

Tokenizer::~Tokenizer()
{
    // Close the file then the tokenizer is destroyed
    fclose(file);
}
