#include "Parser/Class.hpp"
#include "Parser/Function.hpp"
using namespace TinyScript;

void NodeClass::parse_attr(Tokenizer &tk)
{
    DataType type = parse_type(tk);
    Token name = tk.match(TokenType::Name, "Attr name");
    Logger::log(name.debug_info, "Parsing attr '" + name.data + "'");

    int size = DataType::find_size(type);
    int location = allocate(size);
    Symbol attr(name.data, type, SYMBOL_LOCAL, location);
    push_symbol(attr);
    construct->attrs.push(attr);
    construct->size += size;
}

void NodeClass::parse(Tokenizer &tk)
{
    tk.match(TokenType::Class, "class");
    name = tk.match(TokenType::Name, "Class Name");
    construct = get_parent(NodeType::Module)->create_construct(name.data);
    
    Logger::log(name.debug_info, "Parsing class '" + name.data + "'");
    Logger::start_scope();
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
    Logger::end_scope();
}
