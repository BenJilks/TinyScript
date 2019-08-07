#include "Parser/Class.hpp"
#include "Parser/Function.hpp"
using namespace TinyScript;

void NodeClass::parse_attr(Tokenizer &tk)
{
    
}

void NodeClass::parse(Tokenizer &tk)
{
    tk.match(TokenType::Class, "class");
    name = tk.match(TokenType::Name, "Class Name");
    
    tk.match(TokenType::OpenBlock, "{");
    while (tk.get_look().type != TokenType::CloseBlock)
    {
        switch (tk.get_look().type)
        {
            case TokenType::Func: parse_child<NodeFunction>(tk); break;
            default: parse_attr(tk); break;
        }
    }
    tk.match(TokenType::CloseBlock, "}");
}
