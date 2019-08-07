#include "Parser/Expression.hpp"
#include <algorithm>
#include <string.h>
#include <ctype.h>
using namespace TinyScript;

void NodeExpression::parse_args(Tokenizer &tk, ExpDataNode *node)
{
    // Only parse args is there is any provided
    if (tk.get_look().type == TokenType::OpenArg)
    {
        // Loop through all the arguments in a function call
        tk.match(TokenType::OpenArg, "(");
        while (tk.get_look().type != TokenType::CloseArg)
        {
            // Parse the NodeExpression and add it to the nodes list of args
            ExpDataNode *arg = parse_expression(tk);
            node->args.push_back(arg);

            if (tk.get_look().type != TokenType::CloseArg)
                tk.match(TokenType::Next, ",");
        }
        tk.match(TokenType::CloseArg, ")");
    }
}

ExpDataNode *NodeExpression::parse_indies(Tokenizer &tk, ExpDataNode *node)
{
    // Create a new index operation
    // left     - array
    // right    - index

    Token open = tk.match(TokenType::OpenIndex, "[");
    ExpDataNode *index = parse_expression(tk);
    ExpDataNode *operation = new ExpDataNode;
    operation->left = node;
    operation->right = index;
    operation->flags = NODE_INDEX;
    operation->token = open;
    tk.match(TokenType::CloseIndex, "]");

    return operation;
}

static ExpDataNode *cast(ExpDataNode *node, DataType to)
{
    // If an array cast
    if (node->type.flags & DATATYPE_ARRAY)
    {
        DataType original = node->type;
        node->type = to;
        node->type.array_type = shared_ptr<DataType>(new DataType(original));
        node->flags |= NODE_TEMP_REF;
        return node;
    }

    ExpDataNode *cast = new ExpDataNode;
    cast->left = node;
    cast->right = new ExpDataNode { nullptr, nullptr, to };
    cast->type = to;
    cast->flags = NODE_CAST;
    cast->token = { "as", TokenType::As, cast->left->token.debug_info };
    return cast;
}

ExpDataNode *NodeExpression::parse_cast(Tokenizer &tk, ExpDataNode *node)
{
    // Create a new cast operation
    // left     - original
    // right    - new type

    Token as = tk.match(TokenType::As, "as");
    DataType type = parse_type(tk);
    bool warning = false;
    if (!DataType::can_cast_to(node->type, type, warning))
    {
        Logger::error(as.debug_info, "Cannot cast from '" + 
            DataType::printout(node->type) + "' to '" + 
            DataType::printout(type) + "'");
    }

    if (warning)
    {
        Logger::warning(as.debug_info, "Possible loss of data casting from '" + 
            DataType::printout(node->type) + "' to '" + 
            DataType::printout(type) + "'");
    }

    return cast(node, type);
}

ExpDataNode *NodeExpression::parse_in(Tokenizer &tk, ExpDataNode *node)
{
    // Find operation info
    Token in = tk.match(TokenType::In, "in");
    Token attr = tk.match(TokenType::Name, "Name");
    SymbolTable &attrs = node->symb.type.construct->attrs;
    
    // Create the right hand node
    ExpDataNode *attr_node = new ExpDataNode { nullptr, nullptr };
    parse_args(tk, attr_node);
    attr_node->token = attr;
    //attr_node->symb = find_symbol(attrs, attr_node);
    attr_node->type = attr_node->symb.type;

    // Create operation node
    ExpDataNode *operation = new ExpDataNode;
    operation->left = node;
    operation->right = attr_node;
    operation->token = in;
    operation->flags = NODE_IN;
    operation->type = attr_node->symb.type;
    return operation;
}

ExpDataNode *NodeExpression::parse_transforms(Tokenizer &tk, ExpDataNode *node)
{
    bool has_next_transform = true;

    // Keep persing transform operation until there is none left
    while (has_next_transform && !tk.is_eof())
    {
        switch (tk.get_look().type)
        {
            case TokenType::In: node = parse_in(tk, node); break;
            case TokenType::OpenIndex: node = parse_indies(tk, node); break;
            case TokenType::As: node = parse_cast(tk, node); break;
            default: has_next_transform = false; break;
        }
    }

    return node;
}

const Symbol &NodeExpression::find_alternet_overload(ExpDataNode *node, 
    Token name, vector<DataType> params)
{
    vector<Symbol> overloads = lookup_all(name.data);
    for (Symbol overload : overloads)
    {
        // Param size must be the same
        if (overload.params.size() != params.size())
            continue;

        // Find if all params can be casted to the target ones
        bool can_cast = true;
        vector<bool> warnings;
        for (int i = 0; i < overload.params.size(); i++)
        {
            DataType param = params[i];
            DataType target = overload.params[i];
            bool warning = false;
            if (!DataType::equal(param, target) && 
                !DataType::can_cast_to(param, target, warning))
            {
                can_cast = false;
                break;
            }
            warnings.push_back(warning);
        }

        if (can_cast)
        {
            // If they can be, modify the args to do so
            for (int i = 0; i < overload.params.size(); i++)
            {
                DataType param = params[i];
                DataType target = overload.params[i];
                if (warnings[i])
                {
                    Logger::warning(node->args[i]->token.debug_info, 
                        "Possible loss of data casting from '" + 
                        DataType::printout(param) + "' to '" + 
                        DataType::printout(target) + "'");
                }

                if (!DataType::equal(param, target))
                    node->args[i] = cast(node->args[i], target);
            }
            return lookup(name.data, overload.params);
        }
    }

    Symbol temp;
    temp.name = name.data;
    temp.params = params;
    Logger::error(name.debug_info, "Could not find a function that matches " + 
        Symbol::printout(temp));
    return SymbolTable::get_null();
}

const Symbol &NodeExpression::find_symbol(ExpDataNode *node)
{
    // Find the names symbol in the table
    Token name = node->token;
    if (node->args.size() > 0)
    {
        // If the node has arguments, then find the symbol with them
        vector<DataType> params;
        params.reserve(node->args.size());
        
        for (ExpDataNode *arg : node->args)
            params.push_back(arg->type);
        const Symbol &symb = lookup(name.data, params);

        // If the symbol could not be found, search of an overload
        if (symb.flags & SYMBOL_NULL)
            return find_alternet_overload(node, name, params);
        
        return symb;
    }

    const Symbol &symb = lookup(name.data);
    if (symb.flags & SYMBOL_NULL)
    {
        Logger::error(name.debug_info, 
            "Could not find symbol '" + 
            name.data + "'");
    }
    return symb;
}

void NodeExpression::parse_name(Tokenizer &tk, ExpDataNode *node)
{
    // Find the tokens name and parse args if there is any
    parse_args(tk, node);
}

ExpDataNode *NodeExpression::parse_sub_expression(Tokenizer &tk, ExpDataNode *node)
{
    ExpDataNode *sub = parse_expression(tk);
    delete node;
    tk.match(TokenType::CloseArg, ")");

    return sub;
}

void NodeExpression::parse_ref(Tokenizer &tk, ExpDataNode *node)
{
    // Parse the value to take a ref of
    ExpDataNode *lnode = parse_expression(tk);
    node->left = lnode;
}

void NodeExpression::parse_copy(Tokenizer &tk, ExpDataNode *node)
{
    ExpDataNode *lnode = parse_expression(tk);
    if (!(lnode->type.flags & DATATYPE_REF))
        Logger::error(node->token.debug_info, "Only a ref value can be copied");

    node->left = lnode;
    node->type = lnode->type;
    node->type.flags ^= DATATYPE_REF;
}

void NodeExpression::parse_negate(Tokenizer &tk, ExpDataNode *node)
{
    // Create an operation of 0 - <value> to negate
    ExpDataNode *lnode = parse_expression(tk);
    node->left = new ExpDataNode { nullptr, nullptr, lnode->type, 0, 
        { "0", lnode->token.type, lnode->token.debug_info } };
    node->right = lnode;
    node->flags |= NODE_OPERATION;
    node->type = lnode->type;
}

void NodeExpression::parse_array(Tokenizer &tk, ExpDataNode *node)
{
    // Make this an array node
    node->flags = NODE_ARRAY;
    DataType arr_type;

    // Read all array items
    while (!tk.is_eof() && tk.get_look().type != TokenType::CloseIndex)
    {
        ExpDataNode *item = parse_expression(tk);
        node->args.push_back(item);

        // If there's more items, then match a next token
        if (tk.get_look().type != TokenType::CloseIndex)
            tk.match(TokenType::Next, ",");
    }

    tk.match(TokenType::CloseIndex, "]");
}

ExpDataNode *NodeExpression::parse_type_name(Tokenizer &tk, ExpDataNode *node)
{
    ExpDataNode *arg = parse_expression(tk);
    string name = DataType::printout(arg->type);
    node->token = { name, TokenType::String };
    node->type = { PrimTypes::type_null(), DATATYPE_ARRAY, name.length() + 1, 
            shared_ptr<DataType>(new DataType { PrimTypes::type_char(), 0 }) };
    free_node(arg);
    return node;
}

ExpDataNode *NodeExpression::parse_type_size(Tokenizer &tk, ExpDataNode *node)
{
    ExpDataNode *arg = parse_expression(tk);
    node->left = arg;
    return node;
}

ExpDataNode *NodeExpression::parse_term(Tokenizer &tk)
{
    // Create a basic term node
    Token term = tk.skip_token();
    ExpDataNode *node = new ExpDataNode;
    node->type = { PrimTypes::type_null(), 0, 0 };
    node->left = nullptr;
    node->right = nullptr;
    node->flags = 0;
    node->token = term;

    // Fill in node data based on type
    switch (term.type)
    {
        case TokenType::Int: node->type = { PrimTypes::type_int(), 0 }; break;
        case TokenType::Float: node->type = { PrimTypes::type_float(), 0 }; break;
        case TokenType::Bool: node->type = { PrimTypes::type_bool(), 0 }; break;
        case TokenType::Char: node->type = { PrimTypes::type_char(), 0 }; break;
        case TokenType::String: node->type = { PrimTypes::type_null(), DATATYPE_ARRAY, term.data.length() + 1, 
            shared_ptr<DataType>(new DataType { PrimTypes::type_char(), 0 }) }; break;
        case TokenType::Name: parse_name(tk, node); break;
        case TokenType::OpenArg: node = parse_sub_expression(tk, node); break;
        case TokenType::OpenIndex: parse_array(tk, node); break;
        case TokenType::Subtract: parse_negate(tk, node); break;
        case TokenType::Ref: parse_ref(tk, node); break;
        case TokenType::Copy: parse_copy(tk, node); break;
        case TokenType::TypeName: node = parse_type_name(tk, node); break;
        case TokenType::TypeSize: node = parse_type_size(tk, node); break;
        default: Logger::error(term.debug_info, "Unexpected token '" + term.data + "', expected NodeExpression");
    }

    // Parse extra term transform operations
    return parse_transforms(tk, node);
}

static auto add_ops = { TokenType::Add, TokenType::Subtract };
static auto mul_ops = { TokenType::Multiply, TokenType::Divide };
static auto logic_ops = { TokenType::MoreThan, TokenType::LessThan, 
    TokenType::MoreThanEquals, TokenType::LessThanEquals, TokenType::Equals };

DataType NodeExpression::parse_operation_type(ExpDataNode *left, ExpDataNode *right, Token op)
{
    DataType l = left->type;
    DataType r = right->type;

    // if the operation is a logical one, then it returns a bool value
    if (std::find(logic_ops.begin(), logic_ops.end(), op.type) != logic_ops.end())
        return { PrimTypes::type_bool(), 0, 0 };

    if (l.construct == PrimTypes::type_int())
    {
        if (r.construct == PrimTypes::type_int()) return l;
        if (r.construct == PrimTypes::type_float()) return r;
        if (r.construct == PrimTypes::type_char()) return l;
    }
    if (l.construct == PrimTypes::type_float())
    {
        if (r.construct == PrimTypes::type_int()) return l;
        if (r.construct == PrimTypes::type_float()) return l;
        if (r.construct == PrimTypes::type_char()) return l;
    }
    if (l.construct == PrimTypes::type_char())
    {
        if (r.construct == PrimTypes::type_int()) return l;
        if (r.construct == PrimTypes::type_float()) return l;
        if (r.construct == PrimTypes::type_char()) return l;
    }

    return l;
}

template<typename Func>
ExpDataNode *NodeExpression::parse_operation(Tokenizer &tk, 
    Func next_term, vector<TokenType> ops)
{
    ExpDataNode *left = next_term();
    while (std::find(ops.begin(), ops.end(), tk.get_look().type) != ops.end())
    {
        Token op = tk.skip_token();
        ExpDataNode *right = next_term();
        DataType type = parse_operation_type(left, right, op);
        left = new ExpDataNode { left, right, type, NODE_OPERATION, op };
    }
    return left;
}

ExpDataNode *NodeExpression::parse_expression(Tokenizer &tk)
{
    auto term = [this, &tk] { return parse_term(tk); };
    auto factor = [this, &tk, &term] { return parse_operation(tk, term, add_ops); };
    auto value = [this, &tk, &factor] { return parse_operation(tk, factor, mul_ops); };
    auto logic = [this, &tk, &value] { return parse_operation(tk, value, logic_ops); };
    return logic();
}

void NodeExpression::parse(Tokenizer &tk)
{
    exp = parse_expression(tk);
}

void NodeExpression::free_node(ExpDataNode *node)
{
    if (node->left != nullptr)
        free_node(node->left);
    
    if (node->right != nullptr)
        free_node(node->right);
    
    for (ExpDataNode *arg : node->args)
        free_node(arg);

    delete node;
}

bool NodeExpression::is_static_value() const
{
    if (exp->flags & NODE_OPERATION)
        return false;
    return true;
}

int NodeExpression::get_int_value() const
{
    return std::atoi(exp->token.data.c_str());
}

void NodeExpression::symbolize_node(ExpDataNode *node)
{
    // Symbolize left an right nodes
    if (node->left != nullptr)
        symbolize_node(node->left);
    if (node->right != nullptr)
        symbolize_node(node->right);

    // Symbolize arguments
    for (ExpDataNode *arg : node->args)
        symbolize_node(arg);

    // Find operation type
    if (node->flags & NODE_OPERATION)
    {
        node->type = parse_operation_type(node->left, 
            node->right, node->token);
        return;
    }

    // Find symbol
    if (node->token.type == TokenType::Name)
    {
        Symbol symb = find_symbol(node);

        // Copy the symbols type data to the node
        node->type = symb.type;
        node->symb = symb;

        // If the symbol is a function call, mark the node as one
        if (symb.flags & SYMBOL_FUNCTION)
            node->flags |= NODE_CALL;
    }

    if (node->token.type == TokenType::Ref)
    {
        // Create the new ref
        ExpDataNode *lnode = node->left;
        if (lnode->type.flags & DATATYPE_ARRAY)
            node->type = *lnode->type.array_type;
        else
            node->type = lnode->type;
        node->type.array_type = shared_ptr<DataType>(new DataType(node->type));
        node->type.flags |= DATATYPE_REF;
    }

    if (node->token.type == TokenType::TypeSize)
        node->type = DataType { PrimTypes::type_int(), 0 };

    if (node->flags & NODE_INDEX)
    {
        symbolize_node(node->left);
        symbolize_node(node->right);
        node->type = *node->left->type.array_type;
    }
    else if (node->token.type == TokenType::OpenIndex)
    {
        DataType arr_type;
        bool found_type = false;
        for (ExpDataNode *node : node->args)
        {
            if (!found_type)
            {
                arr_type = node->type;
                found_type = true;
            }

            if (!DataType::equal(arr_type, node->type))
            {
                Logger::error(node->token.debug_info, 
                    "Arrays must contain only the same type");
            }
        }

        node->type = DataType { PrimTypes::type_null(), DATATYPE_ARRAY, 
            node->args.size(), shared_ptr<DataType>(new DataType(arr_type)) };
    }
}

void NodeExpression::symbolize()
{
    symbolize_node(exp);
}
