#include "Expression.hpp"
#include "bytecode.h"
#include <algorithm>
#include <ctype.h>
#include <memory.h>
using namespace TinyScript;

Expression::Expression(Tokenizer *tk) :
    tk(tk) {}

// Parse an array component of a type
DataType Expression::parse_array_type(SymbolTable &table, DataType of)
{
    // Create a new object to store the type of data the array holds
    shared_ptr<DataType> arr_type(new DataType(of));
    tk->skip_token();
    tk->match(TokenType::OpenIndex, "[");
    
    // Find the size of the array (must be constant)
    ExpNode *size_expr = parse(table);
    if (size_expr->flags & NODE_OPERATION)
        Logger::error(tk->get_debug_info(), "Array size must be a constant value");
    int size = std::atoi(size_expr->token.data.c_str());
    free_node(size_expr);

    // Create the new array type
    Logger::log(tk->get_debug_info(), "Created array of size " + 
        std::to_string(size) + " and type " + of.construct->name);
    return { PrimTypes::type_null(), DATATYPE_ARRAY, size, arr_type };
}

DataType Expression::parse_type(SymbolTable &table)
{
    DataType type = { PrimTypes::type_null(), 0 };

    // Parse special types
    ExpNode *temp;
    switch (tk->get_look().type)
    {
        case TokenType::Auto: 
            type.flags = DATATYPE_AUTO;
            tk->skip_token(); 
            return type;
        
        case TokenType::TypeOf: 
            tk->skip_token();
            temp = parse(table);
            type = temp->type;
            free_node(temp);
            return type;
    }

    while (!tk->is_eof())
    {
        Token name = tk->get_look();
        DataConstruct *construct = table.find_construct(name.data);
        if (construct->name != "null")
        {
            type.construct = construct;
            tk->skip_token();
            continue;
        }
        else
        {
            bool is_data_mod = true;
            switch (name.type)
            {
                case TokenType::Ref: type.array_type = shared_ptr<DataType>(new DataType(type)); type.flags |= DATATYPE_REF; break;
                case TokenType::Array: type = parse_array_type(table, type); break;
                default: is_data_mod = false; break;
            }

            if (is_data_mod)
            {
                tk->skip_token();
                continue;
            }
        }

        break;
    }

    return type;
}

void Expression::parse_args(SymbolTable &table, ExpNode *node)
{
    // Only parse args is there is any provided
    if (tk->get_look().type == TokenType::OpenArg)
    {
        // Loop through all the arguments in a function call
        tk->match(TokenType::OpenArg, "(");
        while (tk->get_look().type != TokenType::CloseArg)
        {
            // Parse the expression and add it to the nodes list of args
            ExpNode *arg = parse(table);
            node->args.push_back(arg);

            if (tk->get_look().type != TokenType::CloseArg)
                tk->match(TokenType::Next, ",");
        }
        tk->match(TokenType::CloseArg, ")");
    }
}

ExpNode *Expression::parse_indies(SymbolTable &table, ExpNode *node)
{
    // Create a new index operation
    // left     - array
    // right    - index

    Token open = tk->match(TokenType::OpenIndex, "[");
    ExpNode *index = parse(table);
    ExpNode *operation = new ExpNode;
    operation->left = node;
    operation->right = index;
    operation->type = *node->type.array_type;
    operation->flags = NODE_INDEX;
    operation->token = open;
    tk->match(TokenType::CloseIndex, "]");

    return operation;
}

static ExpNode *cast(SymbolTable &table, ExpNode *node, DataType to)
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

    ExpNode *cast = new ExpNode;
    cast->left = node;
    cast->right = new ExpNode { nullptr, nullptr, to };
    cast->type = to;
    cast->flags = NODE_CAST;
    cast->token = { "as", TokenType::As, cast->left->token.debug_info };
    return cast;
}

ExpNode *Expression::parse_cast(SymbolTable &table, ExpNode *node)
{
    // Create a new cast operation
    // left     - original
    // right    - new type

    Token as = tk->match(TokenType::As, "as");
    DataType type = parse_type(table);
    if (!DataType::can_cast_to(node->type, type))
    {
        Logger::error(as.debug_info, "Cannot cast from '" + 
            DataType::printout(node->type) + "' to '" + 
            DataType::printout(type) + "'");
    }

    return cast(table, node, type);
}

ExpNode *Expression::parse_in(SymbolTable &table, ExpNode *node)
{
    // Find operation info
    Token in = tk->match(TokenType::In, "in");
    Token attr = tk->match(TokenType::Name, "Name");
    SymbolTable &attrs = node->symb.type.construct->attrs;
    
    // Create the right hand node
    ExpNode *attr_node = new ExpNode { nullptr, nullptr };
    parse_args(table, attr_node);
    attr_node->token = attr;
    attr_node->symb = find_symbol(attrs, attr_node);
    attr_node->type = attr_node->symb.type;

    // Create operation node
    ExpNode *operation = new ExpNode;
    operation->left = node;
    operation->right = attr_node;
    operation->token = in;
    operation->flags = NODE_IN;
    operation->type = attr_node->symb.type;
    return operation;
}

ExpNode *Expression::parse_transforms(SymbolTable &table, ExpNode *node)
{
    bool has_next_transform = true;

    // Keep persing transform operation until there is none left
    while (has_next_transform && !tk->is_eof())
    {
        switch (tk->get_look().type)
        {
            case TokenType::In: node = parse_in(table, node); break;
            case TokenType::OpenIndex: node = parse_indies(table, node); break;
            case TokenType::As: node = parse_cast(table, node); break;
            default: has_next_transform = false; break;
        }
    }

    return node;
}

const Symbol &Expression::find_alternet_overload(SymbolTable &table, ExpNode *node, 
    Token name, vector<DataType> params)
{
    vector<Symbol> overloads = table.lookup_all(name.debug_info, name.data);
    for (Symbol overload : overloads)
    {
        // Param size must be the same
        if (overload.params.size() != params.size())
            continue;

        // Find if all params can be casted to the target ones
        bool can_cast = true;
        for (int i = 0; i < overload.params.size(); i++)
        {
            DataType param = params[i];
            DataType target = overload.params[i];
            if (!DataType::equal(param, target) && 
                !DataType::can_cast_to(param, target))
            {
                can_cast = false;
                break;
            }
        }

        if (can_cast)
        {
            // If they can be, modify the args to do so
            for (int i = 0; i < overload.params.size(); i++)
            {
                DataType param = params[i];
                DataType target = overload.params[i];
                if (!DataType::equal(param, target))
                    node->args[i] = cast(table, node->args[i], target);
            }
            return table.lookup(name.debug_info, name.data, overload.params);
        }
    }

    Symbol temp;
    temp.name = name.data;
    temp.params = params;
    Logger::error(name.debug_info, "Could not find a function that matches " + 
        Symbol::printout(temp));
    return SymbolTable::get_null();
}

Symbol Expression::find_symbol(SymbolTable &table, ExpNode *node)
{
    // Find the names symbol in the table
    Token name = node->token;
    Symbol symb = const_cast<Symbol&>(table.lookup(name.debug_info, name.data));
    if (node->args.size() > 0)
    {
        // If the node has arguments, then find the symbol with them
        vector<DataType> params;
        params.reserve(node->args.size());
        
        for (ExpNode *arg : node->args)
            params.push_back(arg->type);
        symb = table.lookup(name.debug_info, name.data, params, true);

        // If the symbol could not be found, search of an overload
        if (symb.flags & SYMBOL_NULL)
            symb = find_alternet_overload(table, node, name, params);
    }

    return symb;
}

void Expression::parse_name(SymbolTable &table, ExpNode *node)
{
    // Find the tokens name and parse args if there is any
    parse_args(table, node);
    Symbol symb = find_symbol(table, node);

    // Copy the symbols type data to the node
    node->type = symb.type;
    node->symb = symb;

    // If the symbol is a function call, mark the node as one
    if (symb.flags & SYMBOL_FUNCTION)
        node->flags |= NODE_CALL;
}

ExpNode *Expression::parse_sub_expression(SymbolTable &table, ExpNode *node)
{
    ExpNode *sub = parse(table);
    delete node;
    tk->match(TokenType::CloseArg, ")");

    return sub;
}

void Expression::parse_ref(SymbolTable &table, ExpNode *node)
{
    // Parse the value to take a ref of
    ExpNode *lnode = parse(table);
    node->left = lnode;

    // Create the new ref
    if (lnode->type.flags & DATATYPE_ARRAY)
        node->type = *lnode->type.array_type;
    else
        node->type = lnode->type;
    node->type.array_type = shared_ptr<DataType>(new DataType(node->type));
    node->type.flags |= DATATYPE_REF;
}

void Expression::parse_copy(SymbolTable &table, ExpNode *node)
{
    ExpNode *lnode = parse(table);
    if (!(lnode->type.flags & DATATYPE_REF))
        Logger::error(node->token.debug_info, "Only a ref value can be copied");

    node->left = lnode;
    node->type = lnode->type;
    node->type.flags ^= DATATYPE_REF;
}

void Expression::parse_negate(SymbolTable &table, ExpNode *node)
{
    // Create an operation of 0 - <value> to negate
    ExpNode *lnode = parse(table);
    node->left = new ExpNode { nullptr, nullptr, lnode->type, 0, 
        { "0", lnode->token.type, lnode->token.debug_info } };
    node->right = lnode;
    node->flags |= NODE_OPERATION;
    node->type = lnode->type;
}

void Expression::parse_array(SymbolTable &table, ExpNode *node)
{
    // Make this an array node
    node->flags = NODE_ARRAY;
    DataType arr_type;
    bool type_found = false;

    // Read all array items
    while (!tk->is_eof() && tk->get_look().type != TokenType::CloseIndex)
    {
        ExpNode *item = parse(table);
        node->args.push_back(item);

        // If a type has not been set, do so
        if (!type_found)
        {
            arr_type = item->type;
            type_found = true;
        }

        // Check type is consitant 
        if (!DataType::equal(arr_type, item->type))
        {
            Logger::error(tk->get_debug_info(), 
                "Arrays must contain only the same type");
        }

        // If there's more items, then match a next token
        if (tk->get_look().type != TokenType::CloseIndex)
            tk->match(TokenType::Next, ",");
    }

    node->type = DataType { PrimTypes::type_null(), DATATYPE_ARRAY, 
        node->args.size(), shared_ptr<DataType>(new DataType(arr_type)) };
    tk->match(TokenType::CloseIndex, "]");
}

ExpNode *Expression::parse_type_name(SymbolTable &table, ExpNode *node)
{
    ExpNode *arg = parse(table);
    string name = DataType::printout(arg->type);
    node->token = { name, TokenType::String };
    node->type = { PrimTypes::type_null(), DATATYPE_ARRAY, name.length() + 1, 
            shared_ptr<DataType>(new DataType { PrimTypes::type_char(), 0 }) };
    free_node(arg);
    return node;
}

ExpNode *Expression::parse_type_size(SymbolTable &table, ExpNode *node)
{
    ExpNode *arg = parse(table);
    int size = DataType::find_size(arg->type);
    node->token = { std::to_string(size), TokenType::Int };
    node->type = { PrimTypes::type_int(), 0 };
    free_node(arg);
    return node;
}

ExpNode *Expression::parse_term(SymbolTable &table)
{
    // Create a basic term node
    Token term = tk->skip_token();
    ExpNode *node = new ExpNode;
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
        case TokenType::Name: parse_name(table, node); break;
        case TokenType::OpenArg: node = parse_sub_expression(table, node); break;
        case TokenType::OpenIndex: parse_array(table, node); break;
        case TokenType::Subtract: parse_negate(table, node); break;
        case TokenType::Ref: parse_ref(table, node); break;
        case TokenType::Copy: parse_copy(table, node); break;
        case TokenType::TypeName: node = parse_type_name(table, node); break;
        case TokenType::TypeSize: node = parse_type_size(table, node); break;
        default: Logger::error(term.debug_info, "Unexpected token '" + term.data + "', expected expression");
    }

    // Parse extra term transform operations
    return parse_transforms(table, node);
}

static auto add_ops = { TokenType::Add, TokenType::Subtract };
static auto mul_ops = { TokenType::Multiply, TokenType::Divide };
static auto logic_ops = { TokenType::MoreThan, TokenType::LessThan, 
    TokenType::MoreThanEquals, TokenType::LessThanEquals, TokenType::Equals };

DataType Expression::parse_operation_type(ExpNode *left, ExpNode *right, Token op)
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
ExpNode *Expression::parse_operation(SymbolTable &table, Func next_term, 
    vector<TokenType> ops)
{
    ExpNode *left = next_term();
    while (std::find(ops.begin(), ops.end(), tk->get_look().type) != ops.end())
    {
        Token op = tk->skip_token();
        ExpNode *right = next_term();
        DataType type = parse_operation_type(left, right, op);
        left = new ExpNode { left, right, type, NODE_OPERATION, op };
    }
    return left;
}

ExpNode *Expression::parse(SymbolTable &table)
{
    auto term = [this, &table] { return parse_term(table); };
    auto factor = [this, &table, term] { return parse_operation(table, term, add_ops); };
    auto value = [this, &table, factor] { return parse_operation(table, factor, mul_ops); };
    auto logic = [this, &table, value] { return parse_operation(table, value, logic_ops); };
    return logic();
}

void Expression::free_node(ExpNode *node)
{
    if (node->left != nullptr)
        free_node(node->left);
    
    if (node->right != nullptr)
        free_node(node->right);
    
    for (ExpNode *arg : node->args)
        free_node(arg);

    delete node;
}

static char find_op_code(string name)
{
    int i;
    for (i = 0; i < BC_SIZE; i++)
        if (string(bytecode_names[i]) == name)
            return i;
    return -1;
}

static string str_upper(string str)
{
    string out;
    for (char c : str)
        out += toupper(c);
    return out;
}

static void compile_operation(Token op, DataType ltype, DataType rtype, 
    CodeGen &code, SymbolTable &table)
{
    // Find the bytecode name of the operation
    string op_name;
    switch(op.type)
    {
        case TokenType::Add: op_name = "ADD"; break;
        case TokenType::Subtract: op_name = "SUB"; break;
        case TokenType::Multiply: op_name = "MUL"; break;
        case TokenType::Divide: op_name = "DIV"; break;
        case TokenType::MoreThan: op_name = "MORE_THAN"; break;
        case TokenType::LessThan: op_name = "LESS_THAN"; break;
        case TokenType::MoreThanEquals: op_name = "MORE_THAN_EQUALS"; break;
        case TokenType::LessThanEquals: op_name = "MORE_THAN_EQUALS"; break;
        case TokenType::Equals: op_name = "EQUALS"; break;
    }

    // Create the bytecode name string for the types and find the code
    string code_name = "BC_" + op_name + "_" + str_upper(ltype.construct->name) + 
        "_" + str_upper(rtype.construct->name);
    char bytecode = find_op_code(code_name);
    if (bytecode == -1)
    {
        // If that operation has not been implemented
        Logger::error(op.debug_info, "Cannot do operation " + 
            ltype.construct->name + 
            " " + op.data + " " + 
            rtype.construct->name);
        return;
    }

    // Write this final operation to the code
    code.write_byte(bytecode);
}

static void compile_call(const Symbol &symb, ExpNode *node, CodeGen &code, 
    SymbolTable &table)
{
    // Allocate the return space
    int return_size = DataType::find_size(symb.type);
    if (return_size > 0)
    {
        code.write_byte(BC_ALLOC);
        code.write_byte(return_size);
    }

    // Push the arguments onto the stack
    for (int i = node->args.size() - 1; i >= 0; i--)
        Expression::compile_rvalue(node->args[i], code, table);

    // Call the function depending on type
    if (symb.flags & SYMBOL_EXTERNAL)
    {
        // Call external symbol
        code.write_byte(BC_CALL_EXTERNAL);
        code.write_int(symb.location);
    }
    else
    {
        // Call local tinyscript function
        code.write_byte(BC_CALL);
        code.write_call_loc(symb);
    }
}

static void compile_rname(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Token name = node->token;
    Symbol symb = node->symb;

    // Compile function call
    if (node->flags & NODE_CALL)
    {
        compile_call(symb, node, code, table);
        return;
    }

    // Push local to the stack
    if (symb.flags & SYMBOL_LOCAL)
    {
        code.write_byte(BC_LOAD_LOCAL_X);
        code.write_int(DataType::find_size(symb.type));
        code.write_byte(symb.location);
    }
}

static void compile_ref(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Symbol lvalue = Expression::find_lvalue_location(node->left);
    code.write_byte(BC_LOCAL_REF);
    code.write_byte(lvalue.location);
}

static void compile_copy(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Expression::compile_rvalue(node->left, code, table);
    code.write_byte(BC_COPY);
    code.write_byte(DataType::find_size(node->type));
}

static void compile_array(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    // Push all items to the stack
    for (ExpNode *item : node->args)
        Expression::compile_rvalue(item, code, table);
}

static void compile_cast(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Expression::compile_rvalue(node->left, code, table);
    DataType ltype = node->left->type;
    DataType rtype = node->right->type;

    Logger::log(node->token.debug_info, "Cast from " 
        + DataType::printout(ltype) 
        + " to " + DataType::printout(rtype));
    
    // If converting from a ref, don't do anything
    if (!(ltype.flags & DATATYPE_REF))
    {
        string code_name = "BC_CAST_" + str_upper(ltype.construct->name) + 
            "_" + str_upper(rtype.construct->name);
        code.write_byte(find_op_code(code_name));
    }
}

static void compile_rterm(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Token value = node->token;
    const char *str = value.data.c_str();

    // Compile term based on type
    switch(value.type)
    {
        case TokenType::Int: code.write_byte(BC_PUSH_4); code.write_int(atoi(str)); break;
        case TokenType::Float: code.write_byte(BC_PUSH_4); code.write_float(atof(str)); break;
        case TokenType::Bool: code.write_byte(BC_PUSH_1); code.write_byte(value.data == "true" ? 1 : 0); break;
        case TokenType::Char: code.write_byte(BC_PUSH_1); code.write_byte(value.data[0]); break;
        case TokenType::String: code.write_byte(BC_PUSH_X); code.write_string(value.data); break;
        case TokenType::Name: compile_rname(node, code, table); break;
        case TokenType::Ref: compile_ref(node, code, table); break;
        case TokenType::As: compile_cast(node, code, table); break;
        case TokenType::Copy: compile_copy(node, code, table); break;
        case TokenType::OpenIndex: compile_array(node, code, table); break;
    }
}

static void compile_index(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Expression::compile_rvalue(node->left, code, table);
    Expression::compile_rvalue(node->right, code, table);

    int element_size = DataType::find_size(*node->left->type.array_type);
    if (node->left->type.flags & DATATYPE_REF)
    {
        code.write_byte(BC_PUSH_4);
        code.write_int(element_size);
        code.write_byte(BC_MUL_INT_INT);
        code.write_byte(BC_ADD_INT_INT);
        code.write_byte(BC_COPY);
        code.write_byte(DataType::find_size(node->type));
    }
    else
    {
        code.write_byte(BC_GET_ARRAY_INDEX);
        code.write_int(node->left->type.array_size * element_size);
        code.write_int(element_size);
    }
}

static void compile_rin(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Symbol symb = node->right->symb;
    if (symb.flags & SYMBOL_FUNCTION)
    {
        compile_call(symb, node->right, code, table);
        return;
    }
    Expression::compile_rvalue(node->left, code, table);

    int offset = symb.location;
    int size = DataType::find_size(node->right->type);
    int type_size = DataType::find_size(node->left->type);
    if (node->left->type.flags & DATATYPE_REF)
    {
        code.write_byte(BC_PUSH_4);
        code.write_int(offset);
        code.write_byte(BC_ADD_INT_INT);
        code.write_byte(BC_COPY);
        code.write_byte(size);
    }
    else
    {
        code.write_byte(BC_GET_ATTR);
        code.write_int(offset);
        code.write_int(size);
        code.write_int(type_size);
    }
}

void Expression::compile_rvalue(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    // Compile the code for pushing the value on to the stack
    if (node->flags & NODE_OPERATION)
    {
        compile_rvalue(node->left, code, table);
        compile_rvalue(node->right, code, table);
        compile_operation(node->token, node->left->type, 
            node->right->type, code, table);
    }
    else if (node->flags & NODE_INDEX)
    {
        compile_index(node, code, table);
    }
    else if (node->flags & NODE_IN)
    {
        compile_rin(node, code, table);
    }
    else
    {
        compile_rterm(node, code, table);
    }
    
    // If the node is a temp ref, then copy the value 
    // into that temp var and push the ref to it
    if (node->flags & NODE_TEMP_REF)
    {
        // Find the ref type
        DataType type = *node->type.array_type;

        // Create the temp symbol
        int size = DataType::find_size(type);
        int location = table.allocate(size);
        Symbol temp = Symbol("{temp}", type, SYMBOL_LOCAL, location);

        // Store it and push the location
        code.write_byte(BC_STORE_LOCAL_X);
        code.write_int(size);
        code.write_byte(location);
        code.write_byte(BC_LOCAL_REF);
        code.write_byte(location);
    }
}

bool Expression::is_static_lvalue(ExpNode *node)
{
    if (node->token.type == TokenType::Name)
    {
        Symbol symb = node->symb;
        if (!(symb.flags & SYMBOL_FUNCTION))
            return true;
    }

    return false;
}

Symbol Expression::find_lvalue_location(ExpNode *node)
{
    if (node->token.type != TokenType::Name)
        Logger::error(node->token.debug_info, "Not a valid lvalue");
    return node->symb;
}

static void compile_lname(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Token name = node->token;
    Symbol symb = node->symb;

    if (symb.type.flags & DATATYPE_REF)
    {
        code.write_byte(BC_LOAD_LOCAL_4);
        code.write_byte(symb.location);
        return;
    }

    if (symb.flags & SYMBOL_LOCAL)
    {
        code.write_byte(BC_LOCAL_REF);
        code.write_byte(symb.location);
    }
}

static void compile_lterm(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    if (node->token.type == TokenType::Name)
        compile_lname(node, code, table);
    else
        compile_rterm(node, code, table);
}

static void compile_lin(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    Symbol attr = node->right->symb;
    
    Expression::compile_lvalue(node->left, code, table);
    code.write_byte(BC_PUSH_4);
    code.write_int(attr.location);
    code.write_byte(BC_ADD_INT_INT);
}

void Expression::compile_lvalue(ExpNode *node, CodeGen &code, SymbolTable &table)
{
    if (node->flags & NODE_OPERATION)
    {
        compile_lvalue(node->left, code, table);
        compile_lvalue(node->right, code, table);
        compile_operation(node->token, node->left->type, 
            node->right->type, code, table);
        return;
    }

    if (node->flags & NODE_INDEX)
    {
        compile_lvalue(node->left, code, table);
        compile_rvalue(node->right, code, table);

        int element_size = DataType::find_size(*node->left->type.array_type);
        code.write_byte(BC_PUSH_4);
        code.write_int(element_size);
        code.write_byte(BC_MUL_INT_INT);
        code.write_byte(BC_ADD_INT_INT);
        return;
    }

    if (node->flags & NODE_IN)
    {
        compile_lin(node, code, table);
        return;
    }

    compile_lterm(node, code, table);
}

void Expression::assign_symb(Symbol symb, CodeGen &code)
{
    code.write_byte(BC_STORE_LOCAL_X);
    code.write_int(DataType::find_size(symb.type));
    code.write_byte(symb.location);
}
