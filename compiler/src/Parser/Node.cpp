#include "Parser/Node.hpp"
#include "Parser/Expression.hpp"
using namespace TinyScript;

Node *Node::get_parent(NodeType type)
{
    if (parent == nullptr || get_type() == type)
        return this;
    
    return parent->get_parent(type);
}

string Node::get_prefix() const
{
    string out = prefix;
    if (parent != nullptr)
        out = parent->get_prefix() + out;
    
    return out;
}

vector<Symbol> Node::lookup_all(string name) const
{
    vector<Symbol> out = table.lookup_all(name);
    if (parent != nullptr)
        for (Symbol symb : parent->lookup_all(name))
            out.push_back(symb);
    return out;
}

const Symbol &Node::lookup(string name) const
{
    const Symbol &symb = table.lookup(name);
    if (symb.flags & SYMBOL_NULL && parent != nullptr)
        return parent->lookup(name);
    return symb;
}

const Symbol &Node::lookup(string name, vector<DataType> params) const
{
    const Symbol &symb = table.lookup(name, params);
    if (symb.flags & SYMBOL_NULL && parent != nullptr)
        return parent->lookup(name, params);
    return symb;
}

void Node::push_symbol(Symbol symb)
{
    if (lookup(symb.name, symb.params).flags & SYMBOL_NULL)
        table.push(symb);
}

DataConstruct *Node::find_construct(string name)
{
    auto construct = table.find_construct(name);
    if (construct == PrimTypes::type_null() && parent != nullptr)
        return parent->find_construct(name);
    return construct;
}

NodeBlock::~NodeBlock()
{
    for (Node * child : children)
        delete child;
}

// Parse an array component of a type
DataType Node::parse_array_type(Tokenizer &tk, DataType of)
{
    // Create a new object to store the type of data the array holds
    shared_ptr<DataType> arr_type(new DataType(of));
    tk.skip_token();
    tk.match(TokenType::OpenIndex, "[");
    
    // Find the size of the array (must be constant)
    NodeExpression *size_expr = parse_node<NodeExpression>(tk);
    if (!size_expr->is_static_value())
        Logger::error(tk.get_debug_info(), "Array size must be a constant value");
    int size = size_expr->get_int_value();
    delete size_expr;

    // Create the new array type
    Logger::log(tk.get_debug_info(), "Created array of size " + 
        std::to_string(size) + " and type " + of.construct->name);
    return { PrimTypes::type_null(), DATATYPE_ARRAY, size, arr_type };
}

DataType Node::parse_type(Tokenizer &tk)
{
    DataType type = { PrimTypes::type_null(), 0 };

    // Parse special types
    NodeExpression *temp;
    switch (tk.get_look().type)
    {
        case TokenType::Auto: 
            type.flags = DATATYPE_AUTO;
            tk.skip_token(); 
            return type;
        
        case TokenType::TypeOf: 
            tk.skip_token();
            temp = parse_node<NodeExpression>(tk);
            type = temp->get_data_type();
            delete temp;
            return type;
    }

    while (!tk.is_eof())
    {
        Token name = tk.get_look();
        DataConstruct *construct = find_construct(name.data);
        if (construct->name != "null")
        {
            type.construct = construct;
            tk.skip_token();
            continue;
        }
        else
        {
            bool is_data_mod = true;
            switch (name.type)
            {
                case TokenType::Ref:
                    type = { PrimTypes::type_null(), DATATYPE_REF, 
                        0, std::make_shared<DataType>(type) }; 
                    break;
                
                case TokenType::Array: type = parse_array_type(tk, type); break;
                default: is_data_mod = false; break;
            }

            if (is_data_mod)
            {
                tk.skip_token();
                continue;
            }
        }

        break;
    }

    return type;
}

int Node::allocate(int size)
{
    if (is_allocator)
        return table.allocate(size);
    
    if (parent != nullptr)
        return parent->allocate(size);
    
    Logger::link_error("Could not find allocator node");
    return 0;
}

void Node::copy_node(Node *other)
{
    other->table = table;
    other->is_allocator = is_allocator;
}

void NodeBlock::copy_block(NodeBlock *to)
{
    copy_node(to);
    for (Node *child : children)
        to->add_child(child->copy(to));
}
