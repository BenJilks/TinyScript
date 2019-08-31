#include "Parser/Class.hpp"
#include "Parser/Function.hpp"
using namespace TinyScript;

void NodeClass::parse_attr(Tokenizer &tk)
{
    NodeDataType *type = parse_node<NodeDataType>(tk);
    Token name = tk.match(TokenType::Name, "Attr name");
    attrs.push_back(std::make_pair(type, name));

    Logger::log(name.debug_info, "Parsing attr '" + name.data + "'");
}

void NodeClass::parse(Tokenizer &tk)
{
    tk.match(TokenType::Class, "class");
    name = tk.match(TokenType::Name, "Class Name");
    construct = get_parent(NodeType::Module)->create_construct(name.data);
    construct->parent = this;
    prefix = name.data + ".";
    
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

Node *NodeClass::copy(Node *parent)
{
    NodeClass *other = new NodeClass(parent);
    copy_block(other);
    other->name = name;
    other->construct = construct;
    return other;
}

void NodeClass::register_class()
{
    if (construct->is_complete)
        return;

    Logger::log({}, "Registering type '" + name.data + "'");
    Logger::start_scope();
    
    for (auto attr : attrs)
    {
        NodeDataType *type_node = attr.first;
        Token name = attr.second;

        // Find type and register it if it's not complete
        DataType type = type_node->compile();
        Node *type_parent = type.construct->parent;
        if (type_parent != nullptr)
        {
            NodeClass *type_class = (NodeClass*)type_parent;
            type_class->register_class();
        }

        // Create symbol for each attribute
        int size = DataType::find_size(type);
        int location = allocate(size);
        push_symbol(Symbol(name.data, type, SYMBOL_LOCAL, location, this));
        construct->size += size;

        Logger::log({}, "Registered attr '" + name.data + "' of type '" + 
            DataType::printout(type) + "' size " + std::to_string(size));
    }
    construct->is_complete = true;

    Logger::end_scope();
}

NodeClass::~NodeClass()
{
    // Clean up
    for (auto attr : attrs)
        delete attr.first;
}
