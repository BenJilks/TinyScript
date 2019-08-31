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
