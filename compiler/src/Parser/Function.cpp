#include "Parser/Function.hpp"
#include "Parser/Module.hpp"
using namespace TinyScript;

void NodeLet::parse(Tokenizer &tk)
{
    // Parse let
    tk.match(TokenType::Let, "let");
    name = tk.match(TokenType::Name, "name");
    use_static_type = false;
    value = nullptr;

    if (tk.get_look().type == TokenType::Of)
    {
        tk.match(TokenType::Of, ":");
        static_type = parse_type(tk);
        use_static_type = true;
    }

    if (!use_static_type || tk.get_look().type == TokenType::Assign)
    {
        tk.match(TokenType::Assign, "=");
        value = parse_node<NodeExpression>(tk);
    }

    Logger::log(name.debug_info, "Created new var '" + 
        name.data + "'");
}

void NodeLet::symbolize()
{
    // Create new var symbol
    DataType type = use_static_type ? 
        static_type : value->get_data_type();
    int size = DataType::find_size(type);
    int location = allocate(size);
    
    if (use_static_type && value != nullptr)
    {
        if (!DataType::equal(type, value->get_data_type()))
        {
            Logger::error(value->get_data()->token.debug_info, 
                "Cannot assign '" + DataType::printout(value->get_data_type()) + 
                "' to '" + DataType::printout(type) + "'");
        }
    }

    symb = Symbol(name.data, type, SYMBOL_LOCAL, location);
        get_parent()->push_symbol(symb);
}

Node *NodeLet::copy(Node *parent)
{
    NodeLet *other = new NodeLet(parent);
    other->name = name;
    other->symb = symb;
    other->value = (NodeExpression*)value->copy(other);
    other->static_type = static_type;
    other->use_static_type = use_static_type;
    return other;
}

NodeLet::~NodeLet()
{
    delete value;
}

void NodeAssign::parse(Tokenizer &tk)
{
    Logger::log(tk.get_debug_info(), "Parsing assign statement");

    left = parse_node<NodeExpression>(tk);
    right = nullptr;
    if (tk.get_look().type == TokenType::Assign)
    {
        tk.match(TokenType::Assign, "=");
        right = parse_node<NodeExpression>(tk);
    }
}

Node *NodeAssign::copy(Node *parent)
{
    NodeAssign *other = new NodeAssign(parent);
    other->left = (NodeExpression*)left->copy(other);

    if (right != nullptr)
        other->right = (NodeExpression*)right->copy(other);
    else
        other->right = nullptr;
    return other;
}

NodeAssign::~NodeAssign()
{
    delete left;
    if (right != nullptr)
        delete right;
}

void NodeReturn::parse(Tokenizer &tk)
{
    tk.match(TokenType::Return, "return");
    value = parse_node<NodeExpression>(tk);
}

Node *NodeReturn::copy(Node *parent)
{
    NodeReturn *other = new NodeReturn(parent);
    other->value = (NodeExpression*)value->copy(other);
    return other;
}

NodeReturn::~NodeReturn()
{
    delete value;
}

void NodeIf::parse(Tokenizer &tk)
{
    tk.match(TokenType::If, "if");
    condition = parse_node<NodeExpression>(tk);
    parse_block(tk);
}

Node *NodeIf::copy(Node *parent)
{
    NodeIf *other = new NodeIf(parent);
    copy_block(other);
    other->condition = (NodeExpression*)condition->copy(other);
    return other;
}

NodeIf::~NodeIf()
{
    delete condition;
}

void NodeFor::parse(Tokenizer &tk)
{
    tk.match(TokenType::For, "for");
    left = parse_node<NodeExpression>(tk);
    tk.match(TokenType::Assign, "=");
    from = parse_node<NodeExpression>(tk);
    tk.match(TokenType::To, "to");
    to = parse_node<NodeExpression>(tk);

    parse_block(tk);
}

Node *NodeFor::copy(Node *parent)
{
    NodeFor *other = new NodeFor(parent);
    copy_block(other);
    other->left = (NodeExpression*)left->copy(other);
    other->from = (NodeExpression*)from->copy(other);
    other->to = (NodeExpression*)to->copy(other);
    return other;
}

NodeFor::~NodeFor()
{
    delete left;
    delete from;
    delete to;
}

void NodeWhile::parse(Tokenizer &tk)
{
    tk.match(TokenType::While, "while");
    condition = parse_node<NodeExpression>(tk);
    parse_block(tk);
}

Node *NodeWhile::copy(Node *parent)
{
    NodeWhile *other = new NodeWhile(parent);
    copy_block(other);
    other->condition = (NodeExpression*)condition->copy(other);
    return other;
}

NodeWhile::~NodeWhile()
{
    delete condition;
}

void NodeCodeBlock::parse_statement(Tokenizer &tk)
{
    switch (tk.get_look().type)
    {
        case TokenType::Let: parse_child<NodeLet>(tk); break;
        case TokenType::Return: parse_child<NodeReturn>(tk); break;
        case TokenType::If: parse_child<NodeIf>(tk); break;
        case TokenType::For: parse_child<NodeFor>(tk); break;
        case TokenType::While: parse_child<NodeWhile>(tk); break;
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

    is_template_flag = false;
    vector<DataType> params = parse_params(tk);
    DataType return_type = parse_return_type(tk);

    // Create function symbol
    int flags = SYMBOL_FUNCTION;
    if (is_template_flag)
        flags |= SYMBOL_TEMPLATE;
    symb = Symbol(name.data, return_type, flags, 0);
    symb.params = params;
    symb.parent = this;
    if (get_parent() != nullptr)
        get_parent()->push_symbol(symb);

    // Parse function body
    Logger::start_scope();
    parse_block(tk);
    Logger::end_scope();
}

vector<DataType> NodeFunction::parse_params(Tokenizer &tk)
{
    vector<DataType> param_types;

    tk.match(TokenType::OpenArg, "(");
    while(tk.get_look().type != TokenType::CloseArg && !tk.is_eof())
    {
        // Parse param info
        DataType type = parse_type(tk);
        Token name = tk.match(TokenType::Name, "name");
        param_types.push_back(type);
        if (type.flags & DATATYPE_AUTO)
            is_template_flag = true;

        // Create and allocate new symbol
        int size = DataType::find_size(type);
        Symbol symb(name.data, type, SYMBOL_LOCAL, 0);
        params.push_back(symb);

        Logger::log(name.debug_info, "Found param '" + 
            name.data + "' of type '" + 
            DataType::printout(type) + "'");

        if (tk.get_look().type != TokenType::CloseArg)
            tk.match(TokenType::Next, ",");
    }

    tk.match(TokenType::CloseArg, ")");
    return param_types;
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

const Symbol &NodeFunction::implement(vector<DataType> params)
{
    NodeModule *mod = (NodeModule*)get_parent(NodeType::Module);
    NodeFunction *imp = (NodeFunction*)copy(mod);

    imp->symb = Symbol(imp->name.data, symb.type, SYMBOL_FUNCTION, 0);
    imp->symb.params = params;
    for (int i = 0; i < params.size(); i++)
        imp->params[i].type = params[i];
    imp->is_template_flag = false;
    Logger::log(name.debug_info, "implementing '" + 
        Symbol::printout(imp->symb) + "'");

    mod->add_child(imp);
    mod->push_symbol(imp->symb);
    return mod->lookup(imp->name.data, params);
}

Node *NodeFunction::copy(Node *parent)
{
    NodeFunction *other = new NodeFunction(parent);
    copy_block(other);
    other->name = name;
    other->symb = symb;
    other->arg_size = arg_size;
    other->is_template_flag = is_template_flag;
    other->params = params;
    return other;
}

void NodeFunction::symbolize()
{
    int allocator = -8;
    for (Symbol &symb : params)
    {
        int size = DataType::find_size(symb.type);
        allocator -= size;
        arg_size += size;
        symb.location = allocator;
        push_symbol(symb);
    }
}
