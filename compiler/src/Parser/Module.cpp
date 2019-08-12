#include "Parser/Module.hpp"
#include "Parser/Function.hpp"
#include "Parser/Class.hpp"
#include "Parser/Program.hpp"
using namespace TinyScript;

void NodeImportFrom::parse(Tokenizer &tk)
{
    // Parse import from statement
    tk.match(TokenType::From, "from");
    module = tk.match(TokenType::Name, "Module Name");
    tk.match(TokenType::Import, "import");
    attr = tk.match(TokenType::Name, "Attr Name");

    Logger::log(module.debug_info, "Parsing import '" + 
        attr.data + "' from '" + module.data + "'");
}

Node *NodeImportFrom::copy(Node *parent)
{
    NodeImportFrom *other = new NodeImportFrom(parent);
    other->module = module;
    other->attr = attr;
    return other;
}

void NodeImportFrom::symbolize()
{
    NodeProgram *program = (NodeProgram*)get_parent(NodeType::Program);
    NodeModule *mod = program->load_module(module.data);
    if (mod == nullptr)
        return;

    vector<Symbol> attrs = mod->lookup_all(attr.data);
    if (attrs.size() == 0)
        Logger::error(attr.debug_info, "Could not find symbol '" + 
            attr.data + "'");
    
    for (Symbol symb : attrs)
        get_parent(NodeType::Module)->push_symbol(symb);
}

void NodeImport::parse(Tokenizer &tk)
{
    tk.match(TokenType::Import, "import");
    module = tk.match(TokenType::Name, "Module Name");
}

Node *NodeImport::copy(Node *parent)
{
    NodeImport *other = new NodeImport(parent);
    other->module = module;
    return other;
}

void NodeImport::symbolize()
{
    NodeProgram *program = (NodeProgram*)get_parent(NodeType::Program);
    NodeModule *mod = program->load_module(module.data);
    if (mod == nullptr)
        return;

    Symbol symb(module.data, {}, SYMBOL_MODULE, 0);
    symb.parent = mod;
    get_parent(NodeType::Module)->push_symbol(symb);
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

Node *NodeExtern::copy(Node *parent)
{
    NodeExtern *other = new NodeExtern(parent);
    other->symb = symb;
    return other;
}

void NodeModule::parse(Tokenizer &tk)
{
    Logger::start_scope();

    while (!tk.is_eof())
    {
        switch(tk.get_look().type)
        {
            case TokenType::Func: parse_child<NodeFunction>(tk); break;
            case TokenType::From: parse_child<NodeImportFrom>(tk); break;
            case TokenType::Import: parse_child<NodeImport>(tk); break;
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

Node *NodeModule::copy(Node *parent)
{
    NodeModule *other = new NodeModule(parent);
    copy_block(other);
    other->name = name;
    return other;
}

bool NodeModule::is_compiled() const
{
    if (!is_compiled_flag)
        return false;

    for (Node *child : children)
    {
        if (child->get_type() == NodeType::Function)
        {
            NodeFunction *func = (NodeFunction*)child;
            if (!func->is_compiled() && !func->is_template())
                return false;
        }
    }

    return true;
}
