#include "Parser/Module.hpp"
#include "Parser/Function.hpp"
#include "Parser/Class.hpp"
#include "Parser/Program.hpp"
using namespace TinyScript;

void NodeImport::parse(Tokenizer &tk)
{
    tk.match(TokenType::From, "from");
    module = tk.match(TokenType::Name, "Module Name");
    tk.match(TokenType::Import, "import");
    attr = tk.match(TokenType::Name, "Attr Name");

    Logger::log(module.debug_info, "Parsing import '" + 
        attr.data + "' from '" + module.data + "'");
}

void NodeExtern::parse(Tokenizer &tk)
{
    tk.match(TokenType::Extern, "extern");
    Token name = tk.match(TokenType::Name, "name");
    vector<DataType> params;
    DataType return_type = { PrimTypes::type_null(), 0 };

    // Parse type only params
    if (tk.get_look().type == TokenType::OpenArg)
    {
        tk.match(TokenType::OpenArg, "(");
        while (tk.get_look().type != TokenType::CloseArg && !tk.is_eof())
        {
            DataType param = parse_type(tk);
            params.push_back(param);

            if (tk.get_look().type != TokenType::CloseArg)
                tk.match(TokenType::Next, ",");
        }
        tk.match(TokenType::CloseArg, ")");
    }

    // Create external symbol
    symb = Symbol(name.data, return_type, SYMBOL_FUNCTION | SYMBOL_EXTERNAL, rand());
    symb.params = params;
    get_parent(NodeType::Module)->push_symbol(symb);

    Logger::log(name.debug_info, "Parsing extern '" + Symbol::printout(symb) + "'");
}

void NodeImport::symbolize()
{
    NodeProgram *program = (NodeProgram*)get_parent(NodeType::Program);
    NodeModule *mod = program->find_module(module.data);
    if (mod == nullptr)
    {
        Logger::error(module.debug_info, "Could not find module '" + 
            module.data + "'");
        return;
    }

    vector<Symbol> attrs = mod->lookup_all(attr.data);
    if (attrs.size() == 0)
        Logger::error(attr.debug_info, "Could not find symbol '" + 
            attr.data + "'");
    
    for (Symbol symb : attrs)
        get_parent(NodeType::Module)->push_symbol(symb);
}

void NodeModule::parse(Tokenizer &tk)
{
    Logger::start_scope();

    while (!tk.is_eof())
    {
        switch(tk.get_look().type)
        {
            case TokenType::Func: parse_child<NodeFunction>(tk); break;
            case TokenType::From: parse_child<NodeImport>(tk); break;
            case TokenType::Extern: parse_child<NodeExtern>(tk); break;
            case TokenType::Class: parse_child<NodeClass>(tk); break;
            default:
            {
                Token error = tk.skip_token();
                Logger::error(tk.get_debug_info(), 
                    "Unexpected token '" + error.data + "'");
            }
        }
    }

    Logger::end_scope();
}
