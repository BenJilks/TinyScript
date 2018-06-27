#pragma once
#include <string>
using namespace std;

enum class TkType
{
    Name = 0,
    Int,
    Float,
    Char,
    Bool,
    String,
    
    OpenArg,
    CloseArg,
    OpenBlock,
    CloseBlock,
    OpenIndex,
    CloseIndex,
    Assign,
    Next,
    Path,
    Of,
    Add,
    Sub,
    Mul,
    Div,
    Equals,
    GreaterThan,
    LessThan,
    
    Function,
    Return,
    If,
    For,
    While,
    To,
    Class,
    SysCall,
    SysClass,
    Include,
    EndOfFile
};

struct Token
{
    Token() {}
    Token(string str, TkType type) :
        data(str), type(type) {}
    Token(char c, TkType type) :
        type(type) { data += c; }
    
    string data;
    TkType type;
};

class Tokenizer
{
public:
    Tokenizer(string file_path);
    Token Match(string name, TkType type);
    Token NextLook();
    void Error(string msg);
    
    inline Token Look() const { return look; }
    inline TkType LookType() const { return look.type; }
    inline string LookData() const { return look.data; }
    inline bool HasError() const { return has_error; }
    
    ~Tokenizer();
private:
    Token Next();
    char NextChar();
    Token ReadEquals();
    Token ReadName(char c);
    Token ReadNumber(char c);
    Token ReadChar();
    Token ReadString();
    TkType FindKeywords(string buffer);
    void SkipComment();
    
    FILE *file;
    Token look;
    char back_log;

    // Error checking
    bool has_error;
    int line_number;
};
