#include "Parser/Function.hpp"
using namespace TinyScript;

void NodeLet::parse(Tokenizer &tk)
{
    // Parse let
    tk.match(TokenType::Let, "let");
    name = tk.match(TokenType::Name, "name");
    tk.match(TokenType::Assign, "=");
    value = parse_node<NodeExpression>(tk);
    DataType type = value->get_data_type();

    // Create new var symbol
    int size = DataType::find_size(type);
    int location = table.allocate(size);
    symb = Symbol(name.data, type, SYMBOL_LOCAL, location);
    get_parent()->push_symbol(symb);

    Logger::log(name.debug_info, "Created new var '" + 
        name.data + "' of type '" + 
        DataType::printout(type) + "'");
}

NodeLet::~NodeLet()
{
    delete value;
}

void NodeAssign::parse(Tokenizer &tk)
{
    Logger::log(tk.get_debug_info(), "Parsing assign statement");

    left = parse_node<NodeExpression>(tk);
    tk.match(TokenType::Assign, "=");
    right = parse_node<NodeExpression>(tk);
}

NodeAssign::~NodeAssign()
{
    delete left;
    delete right;
}

void NodeCodeBlock::parse_statement(Tokenizer &tk)
{
    switch (tk.get_look().type)
    {
        case TokenType::Let: parse_child<NodeLet>(tk); break;
        default: parse_child<NodeAssign>(tk); break;
    }
}

void NodeCodeBlock::parse_block(Tokenizer &tk)
{
    if (tk.get_look().type == TokenType::OpenBlock)
    {
        tk.match(TokenType::OpenBlock, "{");
        while (tk.get_look().type != TokenType::CloseBlock && !tk.is_eof())
            parse_statement(tk);
        tk.match(TokenType::CloseBlock, "}");

        return;
    }

    parse_statement(tk);
}

void NodeFunction::parse(Tokenizer &tk)
{
    tk.match(TokenType::Func, "func");
    name = tk.match(TokenType::Name, "name");
    Logger::log(name.debug_info, "Parsing function '" + name.data + "'");

    vector<DataType> params = parse_params(tk);
    DataType return_type = parse_return_type(tk);

    // Create function symbol
    Symbol symb(name.data, return_type, SYMBOL_FUNCTION, 0);
    symb.params = params;
    if (get_parent() != nullptr)
        get_parent()->push_symbol(symb);

    // Parse function body
    Logger::start_scope();
    parse_block(tk);
    Logger::end_scope();
}

vector<DataType> NodeFunction::parse_params(Tokenizer &tk)
{
    vector<DataType> params;
    int allocator = -4;

    tk.match(TokenType::OpenArg, "(");
    while(tk.get_look().type != TokenType::CloseArg && !tk.is_eof())
    {
        // Parse param info
        DataType type = parse_type(tk);
        Token name = tk.match(TokenType::Name, "name");
        params.push_back(type);

        // Create and allocate new symbol
        int size = DataType::find_size(type);
        push_symbol(Symbol(name.data, type, SYMBOL_LOCAL, allocator));
        allocator -= size;

        Logger::log(name.debug_info, "Found param '" + 
            name.data + "' of type '" + 
            DataType::printout(type) + "'");

        if (tk.get_look().type != TokenType::CloseArg)
            tk.match(TokenType::Next, ",");
    }

    tk.match(TokenType::CloseArg, ")");
    return params;
}

DataType NodeFunction::parse_return_type(Tokenizer &tk)
{
    if (tk.get_look().type == TokenType::Gives)
    {
        tk.match(TokenType::Gives, "->");
        return parse_type(tk);
    }

    return { PrimTypes::type_null(), 0 };
}
