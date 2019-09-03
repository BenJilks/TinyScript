#include "Parser/DataType.hpp"
#include "Parser/Expression.hpp"
using namespace TinyScript;

void NodeDataType::parse(Tokenizer &tk)
{
    // Parse auto type
    if (tk.get_look().type == TokenType::Auto)
    {
        type_name = tk.match(TokenType::Auto, "auto");
        is_auto_flag = true;
        return;
    }

    // Parse base type name
    type_name = tk.match(TokenType::Name, "Type name");
    
    // Parse all modifiers
    bool has_modifier = true;
    while (has_modifier)
    {
        switch (tk.get_look().type)
        {
            case TokenType::Ref: modifiers.push_back({ tk.skip_token() }); break;
            case TokenType::Array: parse_array(tk); break;
            default: has_modifier = false; break;
        }
    }
}

void NodeDataType::parse_array(Tokenizer &tk)
{
    DataTypeModifier mod;
    mod.name = tk.match(TokenType::Array, "array");

    // Parse array size
    tk.match(TokenType::OpenIndex, "[");
    NodeExpression *size_exp = parse_node<NodeExpression>(tk);
    tk.match(TokenType::CloseIndex, "]");    

    // Must be a static int value so it's known at compile time
    if (!size_exp->is_static_value() || 
        size_exp->get_data_type().construct != PrimTypes::type_int())
    {
        Logger::error(mod.name.debug_info, 
            "Array size must be a static int value");
    }

    mod.size = size_exp->get_int_value();
    modifiers.push_back(mod);
}

Node *NodeDataType::copy(Node *parent)
{
    NodeDataType *other = new NodeDataType(parent);
    other->type_name = type_name;
    other->modifiers = modifiers;
    return other;
}

DataType NodeDataType::compile()
{
    if (is_auto_flag)
        return { PrimTypes::type_null(), DATATYPE_AUTO };

    // Find base construct
    DataConstruct *construct = find_construct(type_name.data);
    if (construct == PrimTypes::type_null())
    {
        Logger::error(type_name.debug_info, 
            "Could not find type '" + type_name.data + "'");
    }

    // Apply modifiers
    DataType type = { construct, 0 };
    for (DataTypeModifier mod : modifiers)
    {
        switch (mod.name.type)
        {
            case TokenType::Ref: type = compile_ref(type); break;
            case TokenType::Array: type = compile_array(type, mod); break;
        }
    }
    return type;
}

DataType NodeDataType::compile_ref(DataType base)
{
    DataType type;
    type.construct = PrimTypes::type_null();
    type.flags = DATATYPE_REF;
    type.sub_type = std::make_shared<DataType>(base);
    return type;
}

DataType NodeDataType::compile_array(DataType base, DataTypeModifier mod)
{
    DataType type;
    type.construct = base.construct;
    type.flags = DATATYPE_ARRAY;
    type.sub_type = std::make_shared<DataType>(base);
    type.array_size = mod.size;
    return type;
}
